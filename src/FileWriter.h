//
// Created by borelset on 11/28/18.
//

#ifndef NC_FILEWRITER_H
#define NC_FILEWRITER_H

#include <cstdio>
#include <string>

class FileWriter{
public:
    FileWriter(const std::string& fileName){
        file = fopen(fileName.c_str(), "w");
        if(file == nullptr){
            printf("file to open:log\n");
            exit(0);
        }
    }
    ~FileWriter(){
        fclose(file);
    }
    int writeData(char* buffer, int length){
        fwrite(buffer, 1, length, file);
    }
private:
    FILE* file;
};

#endif //NC_FILEWRITER_H
