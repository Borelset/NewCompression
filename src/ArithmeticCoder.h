//
// Created by borelset on 12/1/18.
//

#ifndef NC_ARITHMETICCODER_H
#define NC_ARITHMETICCODER_H

#include <stdio.h>
#include <math.h>
#include <cstring>
#include <cstdint>
#include "Type.h"

#define ENCODE_RANGE 256

class ArithmeticCoder{
public:
    ArithmeticCoder(int maxBytes, int maxOutput):maxEncode(maxBytes), maxDecode(maxDecode){

    }

    int encode(char* bytes, int length, char* output, int bufferLength, int* bitLength){
        int top = 1.0;
        int bottom = 0.0;
        double half = 0.5;
        int position = 0;
        int test = bytes[0];

        for(uint16_t j=0; j<ENCODE_RANGE; j++){
            symbolArray[j] = 0;
        }

        for(int i=0; i<length; i++){
            uint16_t test = (uint8_t)bytes[i];
            for(uint16_t j=(uint8_t)bytes[i]; j<ENCODE_RANGE; j++){
                symbolArray[j]++;
            }
        }
        ArithmeticCodeHeader arithmeticCodeHeader;
        for(int i=0; i<256; i++){
            arithmeticCodeHeader.symbolArray[i] = symbolArray[i];
        }
        arithmeticCodeHeader.bytesIncluded = length;
        printf("Compress bytes:%d\n", length);

        /*
        for(uint8_t i = 0; i<ENCODE_RANGE; i++){
            symbolArray[i] = 1;
        }
         */
        //printSymbolArray();
        int bitsToPlus = 0;

        for(int i=0; i<length; i++){
            int range = top - bottom;
            double shard = range / symbolArray[ENCODE_RANGE-1];
            uint8_t symbol = (uint8_t)bytes[i];
            //printf("symbol:%x\n", symbol);
            top = bottom + shard * symbolArray[symbol];
            if(symbol!=0){
                bottom = bottom + shard * symbolArray[symbol-1];
            }else{
                bottom = bottom;
            }

            while(1){
                half = 0.5;
                //printf("new loop\n");
                if(top <= half && bottom < half){
                    position++;
                    for(int j=0; j<bitsToPlus; j++){
                        addBit(output + sizeof(ArithmeticCodeHeader), position, 1);
                        position++;
                    }
                    bitsToPlus = 0;
                    top *= 2;
                    bottom *= 2;
                    //printf("1 low:%f, high:%f\n", bottom, top);
                }else if(top > half && bottom >= half){
                    addBit(output + sizeof(ArithmeticCodeHeader), position, 1);
                    position++;
                    for(int i=0; i<bitsToPlus; i++){
                        position++;
                    }
                    bitsToPlus = 0;
                    top -= half;
                    bottom -= half;
                    top *= 2;
                    bottom *= 2;
                    //printf("2 low:%f, high:%f\n", bottom, top);
                }else{
                    if(i == length-1){
                        addBit(output + sizeof(ArithmeticCodeHeader), position, 1);
                        position++;
                    }else if(top <= 0.75 && bottom >= 0.25){
                        bitsToPlus++;
                        top = 0.5 + (top-0.5)*2;
                        bottom = 0.5 - (0.5-bottom)*2;
                    }
                    /*
                    else{

                        double multiplierTop = half / (top-half);
                        double multiplierBottom = half / (half-bottom);
                        double multiplier = multiplierTop > multiplierBottom ? multiplierBottom : multiplierTop;
                        //printf("multiplier:%f\n", multiplier);
                        top = half + (top-half) * multiplier;
                        bottom = half - (half-bottom) * multiplier;

                    }
                     */
                    //printf("3 low:%f, high:%f\n", bottom, top);

                    break;
                }
            }
            /*
            for(uint8_t j = (uint8_t)bytes[i]; j<ENCODE_RANGE; j++){
                symbolArray[j]++;
            }
             */
        }
        *bitLength = position + sizeof(ArithmeticCodeHeader)*8;
        arithmeticCodeHeader.totalLength = (*bitLength)/8+1;
        printf("CompressedLength:%d\n", arithmeticCodeHeader.totalLength);
        memcpy(output, &arithmeticCodeHeader, sizeof(arithmeticCodeHeader));
    }
    int decode(char* bytes, int length, char* output){
        ArithmeticCodeHeader* header = (ArithmeticCodeHeader*)bytes;
        if(length < header->totalLength){
            return -1;
        }
        for(int i=0; i<256; i++){
            symbolArray[i] = header->symbolArray[i];
        }

        double top = 1.0;
        double bottom = 0.0;
        double upper = 1.0;
        double lower = 0.0;
        int startPosition = 0;
        int endPosition = 0;
        for(int i=0; i < header->bytesIncluded;){
            int index = endPosition / 8;
            int offset = endPosition % 8;
            int base = checkPositive(bytes[index + sizeof(ArithmeticCodeHeader)], offset);
            printf("%x\n", bytes[index]);

            if(base){
                lower += pow(2, (startPosition - endPosition - 1));
            }else{
                upper -= pow(2, (startPosition - endPosition - 1));
            }
            int pos1 = findSymbol(top, bottom, lower);
            int pos2 = findSymbol(top, bottom, upper);
            if(pos1 == pos2){
                output[i] = pos1;
                i++;

                double multiplier = 1 / (upper - lower);
                double bail = lower;
                double posLowerLimit = symbolBoundary(top, bottom, pos1-1);
                double posUpperLimit = symbolBoundary(top, bottom, pos1);
                top = (posUpperLimit - bail) * multiplier;
                bottom = (posLowerLimit - bail) * multiplier;
                upper = 1.0;
                lower = 0.0;

                endPosition++;
                startPosition = endPosition;
            }else{
                endPosition++;
            }
        }
        return 0;
    }

    int decodeLength(char* bytes){
        ArithmeticCodeHeader* header = (ArithmeticCodeHeader*)bytes;
        return header->bytesIncluded;
    }
private:
    void printSymbolArray(){
        for(int i=0; i<ENCODE_RANGE; i++){
            printf("%x : %d\t", i, symbolArray[i]);
        }
        printf("\n");
    }
    void addBit(char* buffer, int position, bool symbol){
        //printf("pos:%d, symbol:%d\n", position, symbol);
        int byteIndex = position/8;
        int bytesoffset = position%8;
        char editer = 0x1 << (7-bytesoffset);
        //printf("editer:%d\n", editer);
        buffer[byteIndex] |= editer;
        /*
        for(int i=0; i<10; i++){
            printf("%x ", buffer[i]);
        }
        printf("\n");
         */
    }
    int findSymbol(double top, double bottom, double value){
        double pos = (value - bottom) / (top - bottom) * symbolArray[ENCODE_RANGE-1];
        for(int i=0; i<ENCODE_RANGE; i++){
            if(symbolArray[i] > pos ){
                return i;
            }
        }
        return ENCODE_RANGE-1;
    }
    double symbolBoundary(double top, double bottom, int symbol){
        return bottom + (top - bottom) * symbolArray[symbol] / symbolArray[ENCODE_RANGE-1];
    }
    int checkPositive(char byte, int position){
        int testValue = 0x1 << (7 - position);
        return byte == (byte | testValue) ? 1 : 0;
    }

    int maxEncode;
    int maxDecode;
    int symbolArray[ENCODE_RANGE] = {0};
};

#endif //NC_ARITHMETICCODER_H
