#include "src/Fitter.h"
#include "src/FileReader.h"
#include "src/ArithmeticCoder.h"
#include "src/FloatCompressor.h"

int main(int argc, char** argv) {
    if(argc < 2){
        exit(0);
    }

/*
    char testStr[] = "ABCDABCf7w9egusducaljsdbcusa9gv9s7dgvbasdubcudsbicuidsaov907sgvbsd7 bb iosdbciuge97egcdubcisdbcisdbcsdcvvwvh78h0sd9ubvaaubvs7sd90vb9a9sd7v";
    ArithmeticCoder arithmeticCoder(10, 10);
    char buffer[200];
    int bits;
    arithmeticCoder.encode(testStr, sizeof(testStr), buffer, 200, &bits);
    printf("bits:%d\n", bits);
    for(int i=0; i<10; i++){
        printf("%x ", buffer[i]);
    }
*/


    CompressionConfig compressionConfig;
    compressionConfig.range = 0.0001;
    compressionConfig.precisionType = PrecisionType::Relative;
    std::string path(argv[1]);
    FileReader fileReader(path);
    int fileSize = fileReader.getFileSize();
    double buffer[fileSize/8];
    fileReader.getData(buffer, fileSize/8);

    double max = buffer[0], min = buffer[0], total = 0.0;
    for(int i=0; i<fileSize/8; i++){
        if(buffer[0] > max) max = buffer[0];
        if(buffer[0] < min) min = buffer[0];
        total += buffer[0];
    }

    FloatCompressor floatCompressor(max - min, total*8 / fileSize);
    FloatHolder floatHolder;
    Fitter fitter(compressionConfig, floatHolder, floatCompressor);
    int bitBufferLength = BitHolderThreeBytes::getRequiredBufferCount(fileSize/8) + sizeof(int);
    char bitBuffer[bitBufferLength];
    fitter.doFit(buffer, fileSize/8, bitBuffer, bitBufferLength);
    int floatBufferLength = floatHolder.getLength();
    char floatBuffer[floatBufferLength];
    floatHolder.copy(floatBuffer);

    std::string newFileName(argv[1]);
    newFileName += ".nc";
    FileWriter fileWriter(newFileName);
    fileWriter.writeData(bitBuffer, bitBufferLength);
    fileWriter.writeData(floatBuffer, floatBufferLength);



/*==
    double testValue = 0.5;
    char* ptr = (char*)&testValue;
    printf("sizeof:%d\n", sizeof(testValue));
    printf("B1:%x, B2:%x, B3:%x, B4:%x, B5:%x, B6:%x, B7:%x, B8:%x\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);

    FloatCompressor floatCompressor(1.0, 1.0);
    floatCompressor.test(0.5);
    char buffer[8];
    int length = 0;
    memset(buffer, 0, sizeof(buffer));
    floatCompressor.compress(1.5, 0.0001, buffer, &length);
    printf("length:%d\n", length);
    for(int i=0; i<8; i++){
        printf("%x ", buffer[i]);
    }
    */

    return 0;
}