#include"FileUtil.h"
#include<assert.h>
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>

using namespace std;
//filename.c_str用于被fopen打开，“ae”模式为追加打开，没有文件则创建一个
AppendFile::AppendFile(string filename) : fptr(fopen(filename.c_str(), "ae")){
    //用户提供缓冲区
    setbuffer(fptr, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile() {fclose(fptr);}

void AppendFile::append(const char *logline, const size_t len){
    size_t n = this->write(logline, len);
    size_t remain = len - n;
    while(remain > 0){
        size_t x = this->write(logline + n, remain);
        if(x == 0){
            int err = ferror(fptr);
            if (err) fprintf(stderr, "AppendFile::append() failed !\n");
            break;
        }
        n += x;
        remain = len - n;
    }
}

void AppendFile::flush() { fflush(fptr); }
//将指定的字符串写入指定的文件中
size_t AppendFile::write(const char *logline, size_t len){
    return fwrite_unlocked(logline, 1, len, fptr);
}

