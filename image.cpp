#include "image.hpp"

#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

#include <FreeImage.h>

FIBITMAP *
convertRGBFtoRGB(FIBITMAP *rgbf)
{
    FIBITMAP *rgb = FreeImage_Allocate(FreeImage_GetWidth(rgbf), FreeImage_GetHeight(rgbf), 24);

    for (int i = 0; i < FreeImage_GetHeight(rgbf); i++) {
        FIRGBF *rowf = (FIRGBF *)FreeImage_GetScanLine(rgbf, i);
        BYTE *row = (BYTE *)FreeImage_GetScanLine(rgb, i);
        for (int j = 0; j < FreeImage_GetWidth(rgbf); j++, rowf++, row += 3) {
            row[FI_RGBA_RED] = rowf->red * 255.0;
            row[FI_RGBA_GREEN] = rowf->green * 255.0;
            row[FI_RGBA_BLUE] = rowf->blue * 255.0;
        }
    }

    return rgb;
}

FIRGBF *
Image::operator()(int x, int y)
{
    FIRGBF *line = getScanLine(y);
    return line + x;
}

Image::Image(const std::string &fname):
    _data(NULL)
{
    int flag = 0;
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

    // check the file signature and deduce its format
    // (the second argument is currently not used by FreeImage)
    fif = FreeImage_GetFileType(fname.c_str(), 0);
    if(fif == FIF_UNKNOWN) {
        // no signature ?
        // try to guess the file format from the file extension
        fif = FreeImage_GetFIFFromFilename(fname.c_str());
    }

    // check that the plugin has reading capabilities ...
    if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        FIBITMAP *tmp = FreeImage_Load(fif, fname.c_str(), flag);
        _data = FreeImage_ConvertToRGBF(tmp);
        FreeImage_Unload(tmp);
    }
}

Image::Image(const Image &other):
    _data(NULL)
{
    _data = FreeImage_Clone(other._data);
}

Image::~Image()
{
    if(_data) FreeImage_Unload(_data);
}

bool
Image::writeToFile(const std::string &fname) const
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    BOOL status = FALSE;

    int flag = 0;
    if(_data) {
        // try to guess the file format from the file extension
        fif = FreeImage_GetFIFFromFilename(fname.c_str());
        if(fif != FIF_UNKNOWN ) {
            // FIBITMAP *tmp = FreeImage_ConvertTo24Bits(_data);
            FIBITMAP *tmp = convertRGBFtoRGB(_data);

            // check that the plugin has sufficient writing and export capabilities ...
            WORD bpp = FreeImage_GetBPP(tmp);
            if(FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp)) {
                status = FreeImage_Save(fif, tmp, fname.c_str(), flag);
            }

            FreeImage_Unload(tmp);
        }
    }

    return (status == TRUE);
}

int
Image::nChannels() const
{
    int bytespp = FreeImage_GetLine(_data) / FreeImage_GetWidth(_data);
    return bytespp / sizeof(float);
}
