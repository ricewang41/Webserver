#pragma once
#include<memory>
#include<string>
#include "FileUtil.h"
#include "noncopyable.h"
#include "MutexLock.h"

class LogFile : noncopyable{
    public:
    LogFile(const std::string& basename, int flusheveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool roolFile();

    private:
    void append_unlocked(const char* logline, int len);

    const std::string basename_;
    const int flusheveryN_;

    int count;
    std::unique_ptr<MutexLock> mutex_;
    std::unique_ptr<AppendFile> file_;
};