//
// Created by borelset on 12/1/18.
//

#ifndef NC_FLOATCOMPRESSOR_H
#define NC_FLOATCOMPRESSOR_H

#include <cstdint>
#include <cmath>
#include <cstdio>

class FloatCompressor{
public:
    FloatCompressor(double _range, double _median):
        range(_range), median(_median)
    {
        long* rangeInLong = (long*)&_range;
        long rangeExpo = (*rangeInLong) << 1 >> 53;
        int rangeExpoSymbol = (*rangeInLong) >> 63;
        requiredRangeExpo *= rangeExpoSymbol;
    }
    void compress(double value, double precision, char* buffer, int* length){
        double normalized = value - median;
        int normManti = mantiBits(normalized);
        long precisionExpo = expo(precision);
        int requiredManti = 0;
        if(requiredRangeExpo - precisionExpo < 0){
            case0++;
            requiredManti = 0;
        }else if(requiredRangeExpo - precisionExpo > normManti){
            case1++;
            requiredManti = normManti;
            totalManti += requiredManti;
            count++;
        }else{
            case2++;
            requiredManti = requiredRangeExpo - precisionExpo;
            totalManti += requiredManti;
            count++;
        }

        uint8_t requiredLength = 2;
        requiredLength += requiredManti%8 ? requiredManti/8+1 : requiredManti/8;
        uint8_t* normalizedPtr = (uint8_t*)&normalized;
        buffer[0] = (requiredLength << 4) + (normalizedPtr[7] >> 4);
        buffer[1] = (normalizedPtr[7] << 4) + (normalizedPtr[6] >> 4);
        for(int i=0; i<requiredLength-2; i++){
            buffer[2+i] = (normalizedPtr[6-i] << 4) + (normalizedPtr[5-i] >> 4);
        }
        *length = requiredLength;
    }

    void test(double value){
        printf("expo: %ld, manti:%d\n", expo(value), mantiBits(value));
    }
    void print(){
        printf("case0:%d, case1:%d, case2:%d\n", case0, case1, case2);
        printf("avenge manti:%f\n", (double)totalManti / count);
    }

private:
    long expo(double value){
        uint8_t* valuePtr = (uint8_t*)&value;
        long expo = valuePtr[7] << 1;
        expo = expo << (4-1);
        expo += valuePtr[6] >> 4;
        expo -= 1023;
        return expo;
    }

    int mantiBits(double value){
        uint8_t* bytes = (uint8_t*)&value;
        for(int i=0; i<6; i++){
            if(bytes[i] == 0){
                continue;
            }else{
                int bits = 0;
                for(int j=0; j<8; j++){
                    if(bytes[i] != (bytes[i] >> j+1 << j+1)){
                        bits = 8-j;
                    }
                }
                return bits + 4 + 8*(5-i);
            }
        }
        for(int i=0; i<4; i++){
            if((bytes[6] >> i+1 << i+1) != bytes[6]){
                return 4-i;
            }
        }
        return 0;
    }
    double range;
    double median;
    long requiredRangeExpo;
    int requiredMantiLength;
    int case0 = 0;
    int case1 = 0;
    int case2 = 0;
    int totalManti = 0;
    int count = 0;
};

#endif //NC_FLOATCOMPRESSOR_H
