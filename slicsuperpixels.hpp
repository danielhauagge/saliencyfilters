#ifndef __SAC_SLICSUPERPIXELS_HPP__
#define __SAC_SLICSUPERPIXELS_HPP__

#include "common.hpp"
#include "image.hpp"
#include "opencl.hpp"

Size superPixelGridSize(const Size &imageSize, int superPixelSpacing);

void slicSuperPixels(OpenCL &opencl,
                     const Size &imgSize, int stride,
                     int superPixelSpacing, int nIters, float relWeight,
                     Memory &img, Memory &clusterCenters, Memory &clusterAssig);

#endif // __SAC_SICSUPERPIXELS_HPP__
