//
// Created by borelset on 11/28/18.
//

#ifndef NC_TYPE_H
#define NC_TYPE_H

#include <cstring>
#include <functional>
#include <vector>
#include <list>

typedef std::function<double(double, double, double, double)> Callbacks;

double m0(double x1, double x2, double x3, double range){
    return x3;
}

double m1(double x1, double x2, double x3, double range){
    return x3 * 2 - x2;
}

double m2(double x1, double x2, double x3, double range){
    return x3 + range;
}

double m3(double x1, double x2, double x3, double range){
    return x3 - range;
}

double m4(double x1, double x2, double x3, double range){
    return x3 * x3 / x2;
}

double m5(double x1, double x2, double x3, double range){
    return x1;
}

std::vector<Callbacks> callbacks = {
        m0, m1, m2, m3, m4, m5
};

class BitHolderThreeBytes{
public:
    static unsigned getRequiredBufferCount(int length){
        if(length % 8 ==0){
            return (length-3) * 3 / 8;
        }else{
            return (length-3) * 3 / 8 + 1;
        }
    }
    BitHolderThreeBytes(){
        memset(holder, 0, sizeof(holder));
    }
    int addRecord(int method){
        if(offset==8){
            return 1;
        }
        switch (offset){
            case 0:
                holder[0] += method << 5;
                break;
            case 1:
                holder[0] += method << 2;
                break;
            case 2:
                holder[0] += method >> 1;
                holder[1] += method << 7;
                break;
            case 3:
                holder[1] += method << 4;
                break;
            case 4:
                holder[1] += method << 1;
                break;
            case 5:
                holder[1] += method >> 2;
                holder[2] += method << 6;
                break;
            case 6:
                holder[2] += method << 3;
                break;
            case 7:
                holder[2] += method;
                break;
        }
        offset++;
        return 0;
    }
    char* getBuffer(){
        return holder;
    }
    unsigned getSize(){
        return sizeof(holder);
    }
    unsigned getOffset(){
        return offset;
    }
private:
    int offset = 0;
    char holder[3];
};

class FloatHolder{
public:
    FloatHolder(){

    }
    void addRecord(char* input, int len){
        for(int i=0; i<len; i++){
            list.push_back(input[i]);
        }
        length += len;
        counter += 1;
    }
    int getLength(){
        return length + sizeof(int);
    }
    int copy(char* buffer){
        int* intPtr = (int*)buffer;
        intPtr[0] = counter;

        int index = 0 + sizeof(int);
        for(auto bufferIter = list.begin(); bufferIter != list.end(); bufferIter++){
            buffer[index] = *bufferIter;
            index++;
        }
    }
private:
    std::list<char> list;
    int length = 0;
    int counter = 0;
};

class MultiplierHolder{
public:
    MultiplierHolder(){}
    void addRecord(int value){
        list.push_back(value);
        counter++;
    }
    void copy(char* buffer){
        int* header = (int*)buffer;
        header[0] = counter;
        int n = 1;
        for(auto iter = list.begin(); iter != list.end(); iter++){
            header[n] = *iter;
        }
    }
    int getLength(){
        return counter * sizeof(int) + sizeof(int);
    }
private:
    std::list<int> list;
    int counter = 0;
};

struct ArithmeticCodeHeader{
    int totalLength;
    int symbolArray[256];
    int bytesIncluded;
};

#endif //NC_TYPE_H
