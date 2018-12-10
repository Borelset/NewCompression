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
#define RESIZING (pow(2, 53))

class Fitter{
public:
    Fitter(const CompressionConfig& cpc, FloatHolder& fh, FloatCompressor& fc, MultiplierHolder& mh, PrecisionHolder& ph) :
        compressionConfig(cpc), floatHolder(fh), floatCompressor(fc), multiplierHolder(mh), precisionHolder(ph)
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

        int hitting = 0, missing = 0, ranging = 0, skip = 0;
        double acceptedRange = compressionConfig.range;
        double fixedAcceptedRange = 0.0;

        std::fstream fstream("log", std::ios::out);

        input[0] = input[0] * RESIZING;
        lastValues[0] = Unpredited(compressionConfig, floatCompressor, floatHolder, input[0]);
        lastValues[0] += floatCompressor.getMedian();
        fstream << "ori:" << input[0] << " pred:" << lastValues[0] << " rate:" << lastValues[0] / input[0] << " type:" << 0 << std::endl;

        input[1] = input[1] * RESIZING;
        lastValues[1] = Unpredited(compressionConfig, floatCompressor, floatHolder, input[1]);
        lastValues[1] += floatCompressor.getMedian();
        fstream << "ori:" << input[1] << " pred:" << lastValues[1] << " rate:" << lastValues[1] / input[1] << " type:" << 0 << std::endl;

        input[2] = input[2] * RESIZING;
        lastValues[2] = Unpredited(compressionConfig, floatCompressor, floatHolder, input[2]);
        lastValues[2] += floatCompressor.getMedian();
        fstream << "ori:" << input[2] << " pred:" << lastValues[2] << " rate:" << lastValues[2] / input[2] << " type:" << 0 << std::endl;

        unsigned requiredBufferCount = BitHolderThreeBytes::getRequiredBufferCount(inputLength);
        BitHolderThreeBytes bitHolderThreeBytes[requiredBufferCount];
        int bitHolderIndex = 0;
        int segmentIndex = 0;
        double pwRange = 0.0;

        acceptedRange = 1e300;
        for(int i=3; i<inputLength; i++){
            if(segmentIndex < 32 && input[i] != 0){
                pwRange = input[i] * RESIZING;
                pwRange *= compressionConfig.range;
                if(acceptedRange > pwRange){
                    acceptedRange = pwRange;
                }
                segmentIndex++;
            }else if(input[i] == 0){
                segmentIndex++;
            } else{
                CompressedPrecision compressedPrecision;
                char* ptr = (char*)&acceptedRange;
                memcpy(&compressedPrecision, ptr+6, sizeof(CompressedPrecision));
                precisionHolder.addRecord(&compressedPrecision);
                segmentIndex = 1;
                pwRange = input[i] * RESIZING;
                pwRange *= compressionConfig.range;
                acceptedRange = input[i]? pwRange : 1e300;
            }
        }
        {

            CompressedPrecision compressedPrecision;
            char* ptr = (char*)&acceptedRange;
            memcpy(&compressedPrecision, ptr+6, sizeof(CompressedPrecision));
            precisionHolder.addRecord(&compressedPrecision);
        }

        for(int i=3; i<inputLength; i++){
            input[i] *= RESIZING;
            CompressedPrecision compressedPrecision;
            if(compressionConfig.precisionType == PrecisionType::Relative){
                acceptedRange = compressionConfig.range * input[i];
                int index = (i - 3) / 32;
                fixedAcceptedRange = precisionHolder.get(index);
            }

            std::string logMessage;
            double minDelta = acceptedRange;
            int methodIndex = -1;
            //fstream << "i=" << i << ", origin data:" << input[i] << std::endl;
            for(int j=0; j< callbacks.size(); j++){
                double methodValue = callbacks[j](lastValues[0], lastValues[1], lastValues[2]);
                //fstream << "m" << j << "=" << methodValue << ", delta=" << fabs(methodValue - input[i]) << ", minDelta=" << fabs(minDelta) << std::endl;
                if(fabs(methodValue - input[i]) <= fabs(minDelta)){
                    methodIndex = j;
                    minDelta = methodValue - input[i];
                }
            }
            //fstream << "best:m" << methodIndex << ", finally delta:" << minDelta << std::endl << "-----------------------------------" << std::endl;

            if(methodIndex == -1) {
                double diffValue = input[i] - lastValues[2] - floatCompressor.getMedian();
                int multiplier = diffValue / fixedAcceptedRange;

                lastValues[0] = lastValues[1];
                lastValues[1] = lastValues[2];

                if(fixedAcceptedRange != 0 && abs(multiplier) < compressionConfig.diffRange){
                    //precisionHolder.addRecord(&compressedPrecision);
                    multiplierHolder.addRecord(multiplier);
                    methodIndex = 6;
                    lastValues[2] = lastValues[2] + multiplier * fixedAcceptedRange;
                    ranging++;

                    fstream << "ori:" << input[i] << "\tpred:" << lastValues[2] << "\trate:" << lastValues[2] / input[i] << "\ttype:" << 1;
                    fstream << "\tmultiplier:" << multiplier << " fixedAcceptedRange:" << fixedAcceptedRange << std::endl;
                }else{
                    //printf("ori:%f, prev:%f, accr:%f\n", input[i], lastValues[2] + floatCompressor.getMedian(), acceptedRange);
                    //printf("\tmultiplier:%d\n", multiplier);
                    //printf("\tdiffExpo:%ld condition:%d\n", diffExpo, condition);

                    methodIndex = 7;
                    double compressed = floatCompressor.compress(input[i], acceptedRange, floatBuffer, &compressedLength);
                    floatHolder.addRecord(floatBuffer, compressedLength);
                    lastValues[2] = compressed;
                    missing++;

                    fstream << "ori:" << input[i] << " pred:" << lastValues[2]+floatCompressor.getMedian() << " rate:" << (lastValues[2]+floatCompressor.getMedian()) / input[i] << " type:" << 0 << std::endl;
                }

            }else{
                lastValues[0] = lastValues[1];
                lastValues[1] = lastValues[2];
                lastValues[2] = input[i] + minDelta;
                hitting++;

                fstream << "ori:" << input[i] << " pred:" << lastValues[2] << " rate:" << lastValues[2] / input[i] << " type:" << 2 << std::endl;
            }

            while(bitHolderThreeBytes[bitHolderIndex].addRecord(methodIndex) == 1){
                memcpy(output+3*bitHolderIndex, bitHolderThreeBytes[bitHolderIndex].getBuffer(), 3);
                bitHolderIndex++;
            }
        }
        printf("hitting=%d, quantting=%d, missing=%d, skip=%d, total=%d\n", hitting, ranging, missing, skip, hitting+missing+ranging);
        fstream.close();
    }
private:
    double Unpredited(CompressionConfig& cc, FloatCompressor& fc, FloatHolder& fh, double value){
        double acceptedRange;
        if(cc.precisionType == PrecisionType::Relative){
            acceptedRange = compressionConfig.range * value;
        }else{
            compressionConfig.range;
        }
        char floatBuffer[sizeof(double)*2];
        int compressedLength;
        double result = floatCompressor.compress(value, acceptedRange, floatBuffer, &compressedLength);
        floatHolder.addRecord(floatBuffer, compressedLength);
        return result;
    }

    double lastValues[MAX_LAST_VALUE_COUNT];

    CompressionConfig compressionConfig;
    FloatHolder& floatHolder;
    FloatCompressor& floatCompressor;
    MultiplierHolder& multiplierHolder;
    PrecisionHolder& precisionHolder;
};

#endif //NC_FITTER_H
