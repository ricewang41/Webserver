#include "LogStream.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <limits>

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

//muduo库 convert函数返回转换的长度
template <typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    while(i != 0){
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    }
    if(value < 0){
        *p++ = '-';
    }
    //插入string的结尾需要的\0后可以进行reverse操作
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template <typename T>
void LogStream::formatInterger(T v){
    if(buffer_.avail() >= kMaxNumericSize){
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(short v){
    *this<<static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) {
  formatInterger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  formatInterger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  formatInterger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  formatInterger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  formatInterger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  formatInterger(v);
  return *this;
}

LogStream& LogStream::operator<<(double v) {
  if (buffer_.avail() >= kMaxNumericSize) {
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
    buffer_.add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(long double v) {
  if (buffer_.avail() >= kMaxNumericSize) {
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
    buffer_.add(len);
  }
  return *this;
}