#ifndef __SAC_SALIENCYFILTER_HPP__
#define __SAC_SALIENCYFILTER_HPP__

#include "common.hpp"
#include "image.hpp"
#include "opencl.hpp"

// Computes saliency measure on super pixels
void saliencyFiltersSP(OpenCL &opencl,
                       const Size &gridSize,
                       Memory &clusterCenters, Memory &saliency,
                       float stdDevUniqueness = 0.25, float stdDevDistribution = 2.0,
                       float k = 6.0);

#endif // __SAC_SALIENCYFILTER_HPP__
