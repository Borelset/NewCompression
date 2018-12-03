//
// Created by borelset on 11/28/18.
//

#ifndef NC_FITTER_H
#define NC_FITTER_H

#include "FileReader.h"
#include "FileWriter.h"
#include "SevenMethods/CompressionConfig.h"
#include "SevenMethods/Type.h"
#include "FloatCompressor.h"
#include <cmath>
#include <cstdio>
#include <fstream>

#define MAX_LAST_VALUE_COUNT 5

class Fitter{
public:
    Fitter(const CompressionConfig& cpc, FloatHolder& fh, FloatCompressor& fc) :
        compressionConfig(cpc), floatHolder(fh), floatCompressor(fc)
    {

    }
    ~Fitter(){

    }
    int doFit(double* input, int inputLength/*number of double, not bytes*/, char* output, int outputLength){
        if(inputLength <= 3){
            return -1;
        }

        char floatBuffer[sizeof(double)];
        int compressedLength = 0;

        int hitting = 0, missing = 0;
        double acceptedRange = compressionConfig.range;

        lastValues[0] = input[0];
        if(compressionConfig.precisionType == PrecisionType::Relative){
            acceptedRange = compressionConfig.range * input[0];
        }
        floatCompressor.compress(input[0], acceptedRange, floatBuffer, &compressedLength);
        floatHolder.addRecord(floatBuffer, compressedLength);

        lastValues[1] = input[1];
        if(compressionConfig.precisionType == PrecisionType::Relative){
            acceptedRange = compressionConfig.range * input[2];
        }
        floatCompressor.compress(input[1], acceptedRange, floatBuffer, &compressedLength);
        floatHolder.addRecord(floatBuffer, compressedLength);

        lastValues[2] = input[2];
        if(compressionConfig.precisionType == PrecisionType::Relative){
            acceptedRange = compressionConfig.range * input[2];
        }
        floatCompressor.compress(input[2], acceptedRange, floatBuffer, &compressedLength);
        floatHolder.addRecord(floatBuffer, compressedLength);

        std::fstream fstream("log", std::ios::out);

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
            fstream << "i=" << i << ", origin data:" << input[i] << std::endl;
            for(int j=0; j< callbacks.size(); j++){
                double methodValue = callbacks[j](lastValues[0], lastValues[1], lastValues[2], acceptedRange);
                fstream << "m" << j << "=" << methodValue << ", delta=" << fabs(methodValue - input[i]) << ", minDelta=" << fabs(minDelta) << std::endl;
                if(fabs(methodValue - input[i]) < fabs(minDelta)){
                    methodIndex = j;
                    minDelta = methodValue - input[i];
                }
            }
            fstream << "best:m" << methodIndex << ", finally delta:" << minDelta << std::endl << "-----------------------------------" << std::endl;
            if(methodIndex == -1) {
                lastValues[0] = lastValues[1];
                lastValues[1] = lastValues[2];
                lastValues[2] = input[i];
                methodIndex = callbacks.size();
                floatCompressor.compress(input[i], acceptedRange, floatBuffer, &compressedLength);
                floatHolder.addRecord(floatBuffer, compressedLength);
                missing++;
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
        printf("hitting=%d, missing=%d, total=%d\n", hitting, missing, hitting+missing);
        fstream.close();
    }
private:
    double lastValues[MAX_LAST_VALUE_COUNT];

    CompressionConfig compressionConfig;
    FloatHolder& floatHolder;
    FloatCompressor& floatCompressor;
};

#endif //NC_FITTER_H
