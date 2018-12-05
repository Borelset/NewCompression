#include "src/Fitter.h"
#include "src/FileReader.h"
#include "src/ArithmeticCoder.h"
#include "src/FloatCompressor.h"

int main(int argc, char** argv) {
    if(argc < 2){
        exit(0);
    }

    char testStr[] = "abcdefgABCDEFG1234567";
    ArithmeticCoder arithmeticCoder(10, 10);
    char buffer[200 + sizeof(ArithmeticCodeHeader)];
    int bits;
    memset(buffer, 0, 200 + sizeof(ArithmeticCodeHeader));
    arithmeticCoder.encode(testStr, sizeof(testStr), buffer, 200, &bits);
    printf("bits:%d\n", bits);
    for(int i=0 + sizeof(ArithmeticCodeHeader); i<200 + sizeof(ArithmeticCodeHeader); i++){
        printf("%x ", buffer[i]);
    }
    ArithmeticCoder arithmeticCoder1(10, 10);
    int decodeLength = arithmeticCoder1.decodeLength(buffer);
    char decodeBuffer[decodeLength+1];
    memset(decodeBuffer, 0, decodeLength+1);
    int ret = arithmeticCoder1.decode(buffer, 200 + sizeof(ArithmeticCodeHeader), decodeBuffer);
    printf("ret:%d\n", ret);
    printf("%s\n", decodeBuffer);



/*
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
    MultiplierHolder multiplierHolder;
    Fitter fitter(compressionConfig, floatHolder, floatCompressor, multiplierHolder);

    int bitBufferLength = BitHolderThreeBytes::getRequiredBufferCount(fileSize/8) + sizeof(int);
    char bitBuffer[bitBufferLength + sizeof(ArithmeticCodeHeader)];
    fitter.doFit(buffer, fileSize/8, bitBuffer, bitBufferLength);

    int floatBufferLength = floatHolder.getLength();
    char floatBuffer[floatBufferLength + sizeof(ArithmeticCodeHeader)];
    floatHolder.copy(floatBuffer);

    int multiplierBufferLength = multiplierHolder.getLength();
    char multiplierBuffer[multiplierBufferLength + sizeof(ArithmeticCodeHeader)];
    multiplierHolder.copy(multiplierBuffer);

    printf("bitLength:%d, floatLength:%d, multiplierLength:%d\n", bitBufferLength, floatBufferLength, multiplierBufferLength);

    std::string newFileName(argv[1]);
    newFileName += ".nc";
    FileWriter fileWriter(newFileName);

    char* bitBufferAC = (char*)malloc(bitBufferLength);
    memset(bitBufferAC, 0, bitBufferLength);
    int bitACLength = 0;

    char* floatBufferAC = (char*)malloc(floatBufferLength);
    memset(floatBufferAC, 0, floatBufferLength);
    int floatACLength = 0;

    char* multiplierBufferAC = (char*)malloc(multiplierBufferLength);
    memset(multiplierBufferAC, 0, multiplierBufferLength);
    int multiplierACLength = 0;

    ArithmeticCoder arithmeticCoder(0, 0);
    arithmeticCoder.encode(bitBuffer, bitBufferLength, bitBufferAC, bitBufferLength, &bitACLength);
    arithmeticCoder.encode(floatBuffer, floatBufferLength, floatBufferAC, floatBufferLength, &floatACLength);
    arithmeticCoder.encode(multiplierBuffer, multiplierBufferLength, multiplierBufferAC, multiplierBufferLength, &multiplierACLength);

    fileWriter.writeData(bitBufferAC, bitACLength/8+1);
    fileWriter.writeData(floatBufferAC, floatACLength/8+1);
    fileWriter.writeData(multiplierBufferAC, multiplierACLength/8+1);

    free(bitBufferAC);
    free(floatBufferAC);
    free(multiplierBufferAC);
    */


/*
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