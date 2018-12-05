//
// Created by borelset on 11/28/18.
//

#ifndef NC_FITTER_H
#define NC_FITTER_H

#include "FileReader.h"
#include "FileWriter.h"
#include "CompressionConfig.h"
#include "Type.h"
#include "FloatCompressor.h"
#include <cmath>
#include <cstdio>
#include <fstream>

#define MAX_LAST_VALUE_COUNT 5

class Fitter{
public:
    Fitter(const CompressionConfig& cpc, FloatHolder& fh, FloatCompressor& fc, MultiplierHolder& mh) :
        compressionConfig(cpc), floatHolder(fh), floatCompressor(fc), multiplierHolder(mh)
    {

    }
    ~Fitter(){

    }
    int doFit(double* input, int inputLength/*number of double, not bytes*/, char* output, int outputLength){
        if(inputLength <= 3){
            return -1;
        }

        char floatBuffer[sizeof(double)*2];
        int compressedLength = 0;

        int hitting = 0, missing = 0, ranging = 0;
        double acceptedRange = compressionConfig.range;

        lastValues[0] = input[0];
        Unpredited(compressionConfig, floatCompressor, floatHolder, input[0]);

        lastValues[1] = input[1];
        Unpredited(compressionConfig, floatCompressor, floatHolder, input[1]);

        lastValues[2] = input[2];
        Unpredited(compressionConfig, floatCompressor, floatHolder, input[2]);

        //std::fstream fstream("log", std::ios::out);

        unsigned requiredBufferCount = BitHolderThreeBytes::getRequiredBufferCount(inputLength);
        BitHolderThreeBytes bitHolderThreeBytes[requiredBufferCount];
        int bitHolderIndex = 0;
        int* bufferLength = (int*)output;
        bufferLength[0] = inputLength-3;
        output += sizeof(int);

        for(int i=3; i<inputLength; i++){
            if(compressionConfig.precisionType == PrecisionType::Relative){
                acceptedRange = compressionConfig.range * input[i];
            }

            std::string logMessage;
            double minDelta = acceptedRange;
            int methodIndex = -1;
            //fstream << "i=" << i << ", origin data:" << input[i] << std::endl;
            for(int j=0; j< callbacks.size(); j++){
                double methodValue = callbacks[j](lastValues[0], lastValues[1], lastValues[2], acceptedRange);
                //fstream << "m" << j << "=" << methodValue << ", delta=" << fabs(methodValue - input[i]) << ", minDelta=" << fabs(minDelta) << std::endl;
                if(fabs(methodValue - input[i]) < fabs(minDelta)){
                    methodIndex = j;
                    minDelta = methodValue - input[i];
                }
            }
            //fstream << "best:m" << methodIndex << ", finally delta:" << minDelta << std::endl << "-----------------------------------" << std::endl;

            if(methodIndex == -1) {
                int multiplier = fabs((input[i] - lastValues[2]) / acceptedRange);
                if( multiplier < compressionConfig.diffRange ){
                    multiplierHolder.addRecord(multiplier);
                    methodIndex = 6;
                    ranging++;
                }else{
                    methodIndex = 7;
                    floatCompressor.compress(input[i], acceptedRange, floatBuffer, &compressedLength);
                    floatHolder.addRecord(floatBuffer, compressedLength);
                    missing++;
                }
                lastValues[0] = lastValues[1];
                lastValues[1] = lastValues[2];
                lastValues[2] = input[i];
            }else{
                lastValues[0] = lastValues[1];
                lastValues[1] = lastValues[2];
                lastValues[2] = input[i] + minDelta;
                hitting++;
            }

            while(bitHolderThreeBytes[bitHolderIndex].addRecord(methodIndex) == 1){
                memcpy(output+3*bitHolderIndex, bitHolderThreeBytes[bitHolderIndex].getBuffer(), 3);
                bitHolderIndex++;
            }

        }
        printf("hitting=%d, missing=%d, ranging=%d, total=%d\n", hitting, missing, ranging, hitting+missing+ranging);
        //fstream.close();
    }
private:
    void Unpredited(CompressionConfig& cc, FloatCompressor& fc, FloatHolder& fh, double value){
        double acceptedRange;
        if(cc.precisionType == PrecisionType::Relative){
            acceptedRange = compressionConfig.range * value;
        }else{
            compressionConfig.range;
        }
        char floatBuffer[sizeof(double)*2];
        int compressedLength;
        floatCompressor.compress(value, acceptedRange, floatBuffer, &compressedLength);
        floatHolder.addRecord(floatBuffer, compressedLength);
    }

    double lastValues[MAX_LAST_VALUE_COUNT];

    CompressionConfig compressionConfig;
    FloatHolder& floatHolder;
    FloatCompressor& floatCompressor;
    MultiplierHolder& multiplierHolder;
};

#endif //NC_FITTER_H
