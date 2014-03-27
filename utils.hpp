#ifndef __SAC_UTILS_HPP__
#define __SAC_UTILS_HPP__

#include "common.hpp"

void writePgm(const std::string &outFName, float *data, const Size &size);
void writePgm(const std::string &outFName, int *data, const Size &size);

#endif // __SAC_UTILS_HPP__
