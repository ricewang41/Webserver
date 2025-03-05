#pragma once
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "LogStream.h"


class Logger {
    public:
    Logger(const char *fileName, int line);
    ~Logger();
    LogStream &stream() { return impl_.stream_;}

    static void setLogFilename(std::string fileName) { logFileName_ = fileName; }
    static std::string getLogFileName() { return logFileName_; }

    private:
    //私有成员变量
    class Impl {
      public:
        Impl(const char *fileName, int line);
        void formatTime();

        LogStream stream_;
        int line_;
        std::string basename_;
    };
    Impl impl_;
    static std::string logFileName_;
};
//使用 LOG 宏时，会创建一个 Logger 对象，并调用其 stream() 方法返回一个 LogStream 对象的引用，从而允许用户通过流操作符向日志中添加信息.
//例如，使用 LOG << "Error occurred"; 可以记录一条错误信息，并自动记录其来源文件和行号.
#define LOG Logger(__FILE__, __LINE__).stream()