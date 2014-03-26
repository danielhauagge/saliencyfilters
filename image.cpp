#include "image.hpp"

#include <FreeImage.h>

FIBITMAP *
convertRGBFtoRGB(const Image &img)
{
    FIBITMAP *rgb = FreeImage_Allocate(img.size().width, img.size().height, 24);

    for (int i = 0; i < img.size().height; i++) {
        const float *rowf = img.scanLine(i);
        BYTE *row = (BYTE *)FreeImage_GetScanLine(rgb, i);
        for (int j = 0; j < img.size().width; j++, rowf += 3, row += 3) {
            row[FI_RGBA_RED] = rowf[0] * 255.0;
            row[FI_RGBA_GREEN] = rowf[1] * 255.0;
            row[FI_RGBA_BLUE] = rowf[2] * 255.0;
        }
    }

    return rgb;
}

Image::Image(const std::string &fname):
    _img(NULL),
    _size(0, 0)
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
        FIBITMAP *tmp8 = FreeImage_Load(fif, fname.c_str(), flag);
        FIBITMAP *tmp32 = FreeImage_ConvertToRGBF(tmp8);
        FreeImage_Unload(tmp8);

        _size.width = FreeImage_GetWidth(tmp32);
        _size.height = FreeImage_GetWidth(tmp32);
        _stride = FreeImage_GetPitch(tmp32);
        _nChannels =  FreeImage_GetLine(tmp32) / (_size.width * sizeof(float));

        _img = new char[_stride * _size.height];

        memcpy(_img, FreeImage_GetScanLine(tmp32, 0), _size.height * _stride);

        FreeImage_Unload(tmp32);
    }
}

Image::Image(const Image &other):
    _img(NULL)
{
    _size = other._size;
    _stride = other._stride;
    _nChannels = other._nChannels;

    _img = new char[_stride * _size.height];

    memcpy(other._img, _img, _size.height * _stride);
}

Image::~Image()
{
    if(_img) delete [] _img;
}

bool
Image::writeToFile(const std::string &fname) const
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    BOOL status = FALSE;

    int flag = 0;
    if(_img) {
        // try to guess the file format from the file extension
        fif = FreeImage_GetFIFFromFilename(fname.c_str());
        if(fif != FIF_UNKNOWN ) {
            // FIBITMAP *tmp = FreeImage_ConvertTo24Bits(_data);
            FIBITMAP *tmp = convertRGBFtoRGB(*this);

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
