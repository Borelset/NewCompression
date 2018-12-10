//
// Created by borelset on 11/28/18.
//

#ifndef NC_FILEREADER_H
#define NC_FILEREADER_H

#include <string>
#include <sys/stat.h>

enum class FileReaderStat{
    Init,
    Opened,
    Closed,
};

class FileReader{
public:
    FileReader(const std::string& _path){
        file = fopen(_path.c_str(), "r");
        if(file == nullptr){
            printf("no file to open:%s\n", _path.c_str());
            exit(0);
        }
        path = _path;
        stats = FileReaderStat::Opened;
    }
    ~FileReader(){
        stats = FileReaderStat::Closed;
        while(fclose(file) != 0);
    }
    unsigned getFileSize(){
        struct stat statbuf;
        stat(path.c_str(), &statbuf);
        return statbuf.st_size;
    }
    int getData(double* buffer, unsigned count){
        return fread(buffer, sizeof(double), count, file);
    }
    int readFile(char* buffer, unsigned count){
        return fread(buffer, 1, count, file);
    }
private:
    std::string path;
    FileReaderStat stats = FileReaderStat::Init;
    FILE* file = nullptr;
};

#endif //NC_FILEREADER_H
