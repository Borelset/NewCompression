//
// Created by borelset on 12/5/18.
//

#include "src/FileReader.h"
#include "src/FileWriter.h"
#include <cstdio>
#include <list>
#include <cmath>
#include <cstring>
#include <fstream>

#define EXCEL_DOUBLEDATA_HEAD (0x0203)
char divider = ',';
ushort excel_begin[] = { 0x0809, 0x0008, 0x0000, 0x0010, 0x0000, 0x0000 };
ushort excel_end[] = { 0x000A, 0x0000 };

struct ExcelHeader{
    ushort type;
    ushort size;
    ushort row;
    ushort col;
    ushort end = '\0';
};

int main(int argc, char** argv){
    FileReader fileReader1(argv[1]);
    FileReader fileReader2(argv[2]);

    if(fileReader1.getFileSize() != fileReader2.getFileSize()){
        printf("ori:%d, compressed:%d\n", fileReader1.getFileSize(), fileReader2.getFileSize());
        printf("size is not equal\n");
        exit(0);
    }

    int size = fileReader1.getFileSize();

    double oriBuffer[size/8];
    fileReader1.getData(oriBuffer, size/8);
    double compressedBuffer[size/8];
    fileReader2.getData(compressedBuffer, size/8);
    char writeBuffer[100];

    std::fstream fstream("statistics", std::ios::out);

    double total = 0.0;
    int count = 0;
    for(int i=0; i<size/8; i++){
        if(oriBuffer[i] == 0){
            continue;
        }
        double delta = compressedBuffer[i] / oriBuffer[i];
        total+=fabs(delta);
        if(fabs(delta - 1) > 0.01){
            printf("i=%d, rate:%f\n", i, delta);
        }
        count++;
        fstream << "ori:" << oriBuffer[i] << " compressed:" << compressedBuffer[i] << " rate:" << delta << std::endl;
    }
    printf("\n");
    printf("delta rate:%f\n", total / count);

}