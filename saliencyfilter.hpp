#ifndef __SAC_SALIENCYFILTER_HPP__
#define __SAC_SALIENCYFILTER_HPP__

#include "common.hpp"
#include "image.hpp"
#include "opencl.hpp"

// Computes saliency measure on super pixels
void saliencyFiltersSP(OpenCL &opencl,
                       const Size &gridSize,
                       Memory &clusterCenters, Memory &saliencySP,
                       float stdDevUniqueness = 0.25, float stdDevDistribution = 2.0,
                       float k = 6.0);

void propagateSaliency(OpenCL &opencl, const Size &imgSize, int imgStride,
                       const Size &gridSize,
                       Memory &img, Memory &clusterAssig,
                       Memory &saliencySP, Memory &saliency,
                       float alpha = 1.0 / 30.0, float beta = 1.0 / 30.0);

#endif // __SAC_SALIENCYFILTER_HPP__
