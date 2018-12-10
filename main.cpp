#include "src/Fitter.h"
#include "src/FileReader.h"
#include "src/ArithmeticCoder.h"
#include "src/FloatCompressor.h"
#include "src/acoder.hxx"
#include "src/ahcoder.hxx"
#include "src/bhcoder.hxx"
#include "src/Type.h"
#include "src/pcoder.hxx"
#include "src/Huffman.h"
#include <cstdlib>
#include <sys/time.h>

int main(int argc, char** argv) {
    if(argc < 2){
        exit(0);
    }
    printf("filename:%s\n", argv[1]);
    if(atoi(argv[2]) == 0){
        struct timeval pre0, pre1, pre2;

        gettimeofday(&pre0, nullptr);
        CompressionConfig compressionConfig;
        printf("range:%f\n", atof(argv[3]));
        compressionConfig.range = atof(argv[3]);
        compressionConfig.precisionType = PrecisionType::Relative;
        std::string path(argv[1]);
        FileReader fileReader(path);
        int fileSize = fileReader.getFileSize();
        double buffer[fileSize/8];
        fileReader.getData(buffer, fileSize/8);

        double max = buffer[0], min = buffer[0], total = 0.0;
        for(int i=0; i<fileSize/8; i++){
            if(buffer[i] > max) max = buffer[i];
            if(buffer[i] < min) min = buffer[i];
            total += buffer[i];
        }

        FloatCompressor floatCompressor(max - min, total*8 / fileSize);
        FloatHolder floatHolder;
        MultiplierHolder multiplierHolder;
        PrecisionHolder precisionHolder(fileSize);
        Fitter fitter(compressionConfig, floatHolder, floatCompressor, multiplierHolder, precisionHolder);

        printf("file size:%d\n", fileSize);
        int bitBufferLength = BitHolderThreeBytes::getRequiredBufferCount(fileSize/8) * 3;
        printf("count:%u\n", BitHolderThreeBytes::getRequiredBufferCount(fileSize/8));
        char bitBuffer[bitBufferLength];
        fitter.doFit(buffer, fileSize/8, bitBuffer, bitBufferLength);

        int floatBufferLength = floatHolder.getLength();
        char floatBuffer[floatBufferLength];
        floatHolder.copy(floatBuffer);

        int multiplierBufferLength = multiplierHolder.getLength();
        char multiplierBuffer[multiplierBufferLength];
        multiplierHolder.copy(multiplierBuffer);

        int precisionBufferLength = precisionHolder.getLength();
        char precisionBuffer[precisionBufferLength];
        precisionHolder.copy(precisionBuffer);

        FileHeader fileHeader;
        fileHeader.doubleDataCount = fileSize / 8;
        fileHeader.bitLength = bitBufferLength;
        fileHeader.floatLength = floatBufferLength;
        fileHeader.multiplierLength = multiplierBufferLength;
        fileHeader.precisionLength = precisionBufferLength;
        fileHeader.floatMedian = total * 8 /fileSize;
        fileHeader.precisionType = compressionConfig.precisionType;
        fileHeader.precision = compressionConfig.range;

        printf("bitLength:%d, multiplierLength:%d, floatLength:%d, precisionLength:%d\n", bitBufferLength, multiplierBufferLength, floatBufferLength, precisionBufferLength);
        printf("size:%f, %f, %f, %f\n", (double)bitBufferLength/1024, (double)multiplierBufferLength/1024, (double)floatBufferLength/1024, (double)precisionBufferLength/1024);
        //gettimeofday(&pre1, nullptr);
        //double pre1Duration = (pre1.tv_sec-pre0.tv_sec) + double(pre1.tv_usec - pre0.tv_usec) / 1000000;
        double sizeInMB = (double)fileSize / 1024 / 1024;
        //printf("pre1 duration:%fs\nspeed1:%f\n", pre1Duration, sizeInMB / pre1Duration);

        /*
        unsigned char* multiplierHuffmanBuffer;
        int newLength = 0;
        int multipliers[multiplierBufferLength / 2];
        uint16_t* uptr = (uint16_t*)multiplierBuffer;
        for(int i = 0; i<multiplierBufferLength / 2; i++){
            multipliers[i] = uptr[i];
        }
        myinit();
        encode_withTree(multipliers, multiplierBufferLength / 2, &multiplierHuffmanBuffer, &newLength);
        mydeinit();
        printf("before:%d, huffmaned length:%d\n", multiplierBufferLength, newLength);
        free(multiplierHuffmanBuffer);
         */

        std::string newFileName(argv[1]);
        newFileName += ".nc";

        {
            FileWriter fileWriter(newFileName);

            fileWriter.writeData((char*)&fileHeader, sizeof(FileHeader));
            fileWriter.writeData(bitBuffer, bitBufferLength);
            fileWriter.writeData(floatBuffer, floatBufferLength);
            fileWriter.writeData(multiplierBuffer, multiplierBufferLength);
            fileWriter.writeData(precisionBuffer, precisionBufferLength);
        }

        std::ifstream ifs;
        ifs.open(newFileName, std::ios::binary);

        std::string compressedFileName(argv[1]);
        compressedFileName += ".ac";
        std::ofstream ofs;
        ofs.open(compressedFileName, std::ios::binary);
        gettimeofday(&pre2, nullptr);
        double pre2Duration = (pre2.tv_sec-pre0.tv_sec) + double(pre2.tv_usec - pre0.tv_usec) / 1000000;
        printf("pre2 duration:%fs\nspeed:%fMB/s\n", pre2Duration, sizeInMB / pre2Duration);


        /*
        struct timeval t0, t1;
        gettimeofday(&t0, nullptr);
        //staticcodes::acoder ac(ifs, ofs);
        gettimeofday(&t1, nullptr);
        printf("ac duration:%fs\n", (t1.tv_sec-t0.tv_sec) + double(t1.tv_usec - t0.tv_usec) / 1000000);

        gettimeofday(&t0, nullptr);
        adaptivecodes::ahcoder h(ifs, ofs);
        gettimeofday(&t1, nullptr);
        printf("ah duration:%fs\n", (t1.tv_sec-t0.tv_sec) + double(t1.tv_usec - t0.tv_usec) / 1000000);

        gettimeofday(&t0, nullptr);
        //staticcodes::pcoder<staticcodes::huffman> ph(ifs, ofs);
        gettimeofday(&t1, nullptr);
        printf("sh duration:%fs\n", (t1.tv_sec-t0.tv_sec) + double(t1.tv_usec - t0.tv_usec) / 1000000);
         */
        

    }else{
/*
        std::ifstream ifs;
        ifs.open(argv[1], std::ios::binary);
        std::ofstream ofs;
        std::string outName(argv[1]);
        ofs.open(outName + ".tmp", std::ios::binary);
        staticcodes::adecoder adc(ifs, ofs);


        FileReader fileReader(outName + ".tmp");
        */
        std::string outName(argv[1]);
        FileReader fileReader(outName);
        FileHeader fileHeader;
        fileReader.readFile((char*)&fileHeader, sizeof(fileHeader));

        char bitBuffer[fileHeader.bitLength];
        fileReader.readFile(bitBuffer, fileHeader.bitLength);
        char floatBuffer[fileHeader.floatLength];
        fileReader.readFile(floatBuffer, fileHeader.floatLength);
        char multiplierBuffer[fileHeader.multiplierLength];
        fileReader.readFile(multiplierBuffer, fileHeader.multiplierLength);
        char precisionBuffer[fileHeader.precisionLength];
        fileReader.readFile(precisionBuffer, fileHeader.precisionLength);

        double floatMedian = fileHeader.floatMedian;
        int doubleCount = fileHeader.doubleDataCount;

        printf("bitLength:%d, multiplierLength:%d, floatLength:%d, precisionLength:%d\n", fileHeader.bitLength, fileHeader.multiplierLength, fileHeader.floatLength, fileHeader.precisionLength);
        printf("size:%f, %f, %f\n", (double)fileHeader.bitLength/1024, (double)fileHeader.multiplierLength/1024, (double)fileHeader.floatLength/1024);

        FileWriter fileWriter(outName + ".dc");
        double lastValue[3] = {0};

        uint8_t* floatPtr = (uint8_t*)floatBuffer;
        int16_t* multiplierPtr = (int16_t*)multiplierBuffer;
        CompressedPrecision* precisionPtr = (CompressedPrecision*)precisionBuffer;

        std::fstream fstream("decompression", std::ios::out);

        int floatLength = 0;
        double doubleBuffer;
        floatLength = FloatCompressor::specialToDouble(floatPtr, &doubleBuffer);
        lastValue[0] = doubleBuffer;
        floatPtr += floatLength;
        fileWriter.writeDouble(doubleBuffer / pow(2,53));
        fstream << "compressed:" << doubleBuffer / pow(2, 53) << " type:0" << std::endl;

        floatLength = FloatCompressor::specialToDouble(floatPtr, &doubleBuffer);
        lastValue[1] = doubleBuffer;
        floatPtr += floatLength;
        fileWriter.writeDouble(doubleBuffer / pow(2,53));
        fstream << "compressed:" << doubleBuffer / pow(2, 53) << " type:0" << std::endl;

        floatLength = FloatCompressor::specialToDouble(floatPtr, &doubleBuffer);
        lastValue[2] = doubleBuffer;
        floatPtr += floatLength;
        fileWriter.writeDouble(doubleBuffer / pow(2,53));
        fstream << "compressed:" << doubleBuffer / pow(2, 53) << " type:0" << std::endl;

        int array[8] = {0};
        for(int i=0; i<100; i++){
            printf("%x ", bitBuffer[i]);
        }
        printf("\n");
        int segmentIndex = 0;
        for(int i=0; i<doubleCount-3; i++){
            int groupIndex = i / 8;
            int groupOffset = i % 8;
            uint8_t* groupPtr = (uint8_t*)bitBuffer + 3 * groupIndex;
            /*
            if(groupOffset == 0){
                printf("groupPtr:%x\n", groupPtr);
                printf("%x%x%x\n", groupPtr[0], groupPtr[1], groupPtr[2]);
            }*/

            uint8_t method = -1;
            uint8_t test;

            switch (groupOffset){
                case 0:
                    method = (groupPtr[0] >> 5);
                    break;
                case 1:
                    method = groupPtr[0] << 3;
                    method = method >> 5;
                    break;
                case 2:
                    method = groupPtr[0] << 6;
                    method = (method >> 5);
                    method += groupPtr[1] >> 7;
                    break;
                case 3:
                    method = (groupPtr[1] << 1);
                    method = method >> 5;
                    break;
                case 4:
                    method = (groupPtr[1] << 4);
                    method = method >> 5;
                    break;
                case 5:
                    method = (groupPtr[1] << 7);
                    method = (method >> 5);
                    method += groupPtr[2] >> 6;
                    break;
                case 6:
                    method = (groupPtr[2] << 2);
                    method = method >> 5;
                    break;
                case 7:
                    method = (groupPtr[2] << 5);
                    method = method >> 5;
                    break;
            }
            array[method]++;
            if(method == 6){
                doubleBuffer = (*multiplierPtr) * PrecisionHolder::CompressedToDouble(precisionPtr) + lastValue[2];
                multiplierPtr++;
                lastValue[0] = lastValue[1];
                lastValue[1] = lastValue[2];
                lastValue[2] = doubleBuffer;
                fileWriter.writeDouble(doubleBuffer / pow(2,53));
                fstream << "compressed:" << doubleBuffer / pow(2, 53) << " type:1" << " multiplier:" << *multiplierPtr << " pre:" << PrecisionHolder::CompressedToDouble(precisionPtr) << std::endl;
            }else if(method == 7){
                floatLength = FloatCompressor::specialToDouble(floatPtr, &doubleBuffer);
                doubleBuffer += floatMedian;
                lastValue[0] = lastValue[1];
                lastValue[1] = lastValue[2];
                lastValue[2] = doubleBuffer;
                floatPtr += floatLength;
                fileWriter.writeDouble(doubleBuffer / pow(2,53));
                fstream << "compressed:" << doubleBuffer / pow(2, 53) << " type:0" << std::endl;
            }else{
                doubleBuffer = callbacks[method](lastValue[0], lastValue[1], lastValue[2]);
                lastValue[0] = lastValue[1];
                lastValue[1] = lastValue[2];
                lastValue[2] = doubleBuffer;
                fileWriter.writeDouble(doubleBuffer / pow(2,53));
                fstream << "compressed:" << doubleBuffer / pow(2, 53) << " type:2" << std::endl;
            }
            if(segmentIndex == 31){
                segmentIndex = 0;
                precisionPtr++;
            }else{
                segmentIndex++;
            }
        }
        for(int i=0; i<8; i++){
            printf("method[%d]:%d\n", i, array[i]);
        }


    }


    return 0;
}