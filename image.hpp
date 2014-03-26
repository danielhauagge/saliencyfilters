#ifndef __SAL_IMAGE_HPP__
#define __SAL_IMAGE_HPP__

#include "common.hpp"

#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

class Image
{
private:
    char *_img;
    Size _size;
    int _stride; // in bytes
    int _nChannels;

public:
    class Error
    {
    public:
        std::string msg;
        Error(const std::string &msg): msg(msg) {}
    };

    Image(const std::string &fname);
    Image(const Image &other);
    Image(const Size &size);
    ~Image();

    bool writeToFile(const std::string &fname) const;

    const Size &size() const { return _size; }
    int stride() const { return _stride; }
    int nChannels() const { return _nChannels; }

    float *scanLine(int y) { return (float *) (_img + _stride * y); }
    const float *scanLine(int y) const { return (float *) (_img + _stride * y); }

    float *operator()(int x, int y) { return scanLine(y) + x * _nChannels; }
    const float *operator()(int x, int y) const { return scanLine(y) + x * _nChannels; }
};

#endif // __SAL_IMAGE_HPP__

