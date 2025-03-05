#include "LogFile.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "FileUtil.h"

using namespace std;

LogFile::LogFile(const std::string& basename, int flusheveryN )
    : basename_(basename),
      flusheveryN_(flusheveryN),
      count(0),
      mutex_(new MutexLock){
        assert(basename.find('/') >= 0);
        file_.reset(new AppendFile(basename));
}

LogFile::~LogFile() {}

void LogFile::append(const char* logline, int len){
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
}
//追加日志，如果计数器大于N就刷新缓冲区，并且重新设置计数器
void LogFile::append_unlocked(const char* logline, int len){
    file_->append(logline, len);
    ++count;
    if (count >= flusheveryN_){
        count = 0;
        file_->flush();
    }
}
//刷新缓冲区（加🔓版）
void LogFile::flush(){
    MutexLockGuard lock(*mutex_);
    file_->flush();
}