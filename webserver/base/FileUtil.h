# pragma once
#include<string>
#include"noncopyable.h"

//继承不可拷贝类
class AppendFile:noncopyable{
    public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    //用append向文件写
    void append(const char *logline, const size_t len);
    void flush();

    private:
    size_t write(const char *logline, size_t len);
    FILE *fptr;
    char buffer_[64 * 1024];

};