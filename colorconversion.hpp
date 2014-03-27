#ifndef __SAC_COLORCONVERSION_HPP__
#define __SAC_COLORCONVERSION_HPP__

#include "common.hpp"
#include "opencl.hpp"

void rgb2lab(OpenCL &opencl, const Size &size, int stride, Memory &rgb, Memory &lab);

#endif // __SAC_COLORCONVERSION_HPP__
