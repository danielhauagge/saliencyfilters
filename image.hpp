#ifndef __SAL_IMAGE_HPP__
#define __SAL_IMAGE_HPP__

#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

#include <FreeImage.h>

FIBITMAP *convertRGBFtoRGB(FIBITMAP *rgbf);

class Image
{
private:
    FIBITMAP *_data;

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

    int width() const { assert(_data != NULL); return FreeImage_GetWidth(_data); }
    int height() const { assert(_data != NULL); return FreeImage_GetHeight(_data); }
    int nChannels() const;

    FIRGBF *getScanLine(int y) { return (FIRGBF *)FreeImage_GetScanLine(_data, y); }
    FIRGBF *operator()(int x, int y);
};

#endif // __SAL_IMAGE_HPP__

