//
// Created by borelset on 12/1/18.
//

#ifndef NC_ARITHMETICCODER_H
#define NC_ARITHMETICCODER_H

#include <stdio.h>
#include <math.h>
#include <cstring>

#define ENCODE_RANGE 256

class ArithmeticCoder{
public:
    ArithmeticCoder(int maxBytes, int maxOutput):maxEncode(maxBytes), maxDecode(maxDecode){}

    int encode(char* bytes, int length, char* output, int bufferLength, int* bitLength){
        double top = 1.0;
        double bottom = 0.0;
        for(int i=0; i<length; i++){
            for(int j=(int)bytes[i]; j<ENCODE_RANGE; j++){
                symbolArray[j]++;
            }
        }
        printSymbolArray();

        for(int i=0; i<length; i++){
            double range = top - bottom;
            double shard = range / symbolArray[ENCODE_RANGE-1];
            int symbol = (int)bytes[i];
            printf("symbol:%x\n", symbol);
            if(i!=0){
                bottom = shard * symbolArray[symbol-1];
            }else{
                bottom = 0.0;
            }
            top = shard * symbolArray[symbol];
            printf("top:%lf, bottom:%lf\n", top, bottom);
            for(int j=symbol; j<ENCODE_RANGE; j++){
                symbolArray[j]++;
            }
            printSymbolArray();
        }
        printf("low:%f, high:%f\n", bottom, top);
        *bitLength = 0;
        memset(output, 0, bufferLength);
        double base = 0.0;
        for(int i=1; i<bufferLength*8; i++){
            double power = pow(2, -i);
            if(top < base+power && bottom < base+power){
                //addBit(output, i-1, 0);
            }else if(top > base+power && bottom < base+power){
                addBit(output, i-1, 1);
                *bitLength = i;
                break;
            }else if(top > base+power && bottom > base+power){
                addBit(output, i-1, 1);
                base += power;
            }
        }
    }
    int decode(char* bytes, int length, char* output, int bufferLength);
private:
    void printSymbolArray(){
        for(int i=0; i<ENCODE_RANGE; i++){
            printf("%x : %d\t", i, symbolArray[i]);
        }
        printf("\n");
    }
    void addBit(char* buffer, int position, bool symbol){
        printf("pos:%d, symbol:%d\n", position, symbol);
        int byteIndex = position/8;
        int bytesoffset = position%8;
        char editer = 0x1 << (7-bytesoffset);
        printf("editer:%d\n", editer);
        buffer[byteIndex] |= editer;
        for(int i=0; i<10; i++){
            printf("%x ", buffer[i]);
        }
        printf("\n");
    }

    int maxEncode;
    int maxDecode;
    int symbolArray[ENCODE_RANGE] = {0};
};

#endif //NC_ARITHMETICCODER_H
