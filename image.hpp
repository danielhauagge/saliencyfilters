#ifndef __SAL_IMAGE_HPP__
#define __SAL_IMAGE_HPP__

#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

class Image
{
private:
    char *_img;
    int _width, _height;
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
    ~Image();

    bool writeToFile(const std::string &fname) const;

    int width() const { return _width; }
    int height() const { return _height; }
    int stride() const { return _stride; }
    int nChannels() const { return _nChannels; }

    float *scanLine(int y) { return (float *) (_img + _stride * y); }
    const float *scanLine(int y) const { return (float *) (_img + _stride * y); }
    float *operator()(int x, int y) { return scanLine(y) + x * sizeof(float) * _nChannels; }
};

#endif // __SAL_IMAGE_HPP__

