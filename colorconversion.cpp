#include "colorconversion.hpp"

static
const char *kernelSources =
    "void                                                                    \n" \
    "matmul(const float m[3][3], const float v[3], float w[3])               \n" \
    "{                                                                       \n" \
    "    for(int i = 0; i < 3; i++) {                                        \n" \
    "        w[i] = 0.0;                                                     \n" \
    "        for(int j = 0; j < 3; j++) {                                    \n" \
    "            w[i] += m[i][j] * v[j];                                     \n" \
    "        }                                                               \n" \
    "    }                                                                   \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "__kernel void rgb2lab(                                                  \n" \
    "    __global float *rgbIm,                                              \n" \
    "    __global float *labIm,                                              \n" \
    "    const int width, const int height, const int stride)                \n" \
    "{                                                                       \n" \
    "    // rgb to xyz transform                                             \n" \
    "    const float m[3][3] = {                                             \n" \
    "        {0.412453, 0.357580, 0.180423},                                 \n" \
    "        {0.212671, 0.715160, 0.072169},                                 \n" \
    "        {0.019334, 0.119193, 0.950227}                                  \n" \
    "    };                                                                  \n" \
    "                                                                        \n" \
    "    int i = get_global_id(0);                                           \n" \
    "    int j = get_global_id(1);                                           \n" \
    "                                                                        \n" \
    "    if(i < height && j < width) {                                       \n" \
    "        int idx = (stride / 4) * i + 3 * j;                             \n" \
    "        float rgb[3] = {rgbIm[idx], rgbIm[idx + 1], rgbIm[idx + 2]};    \n" \
    "                                                                        \n" \
    "        float xyz[3];                                                   \n" \
    "        matmul(m, rgb, xyz);                                            \n" \
    "                                                                        \n" \
    "        xyz[0] /= 0.950456;                                             \n" \
    "        xyz[2] /= 1.088754;                                             \n" \
    "                                                                        \n" \
    "        float thresh = 0.008856;                                        \n" \
    "        int xyzThresh[3];                                               \n" \
    "        for(int ii = 0; ii < 3; ii++) xyzThresh[ii] = xyz[ii] > thresh; \n" \
    "                                                                        \n" \
    "        float xyzF[3];                                                  \n" \
    "        for(int ii = 0; ii < 3; ii++) {                                 \n" \
    "            if(xyzThresh[ii]) xyzF[ii] =  pow(xyz[ii], (float)(1.0f / 3.0f));\n" \
    "            else xyzF[ii] = 7.787 * xyz[ii] + 16.0 / 116.0;             \n" \
    "        }                                                               \n" \
    "                                                                        \n" \
    "        float Lab[3];                                                   \n" \
    "        if(xyzThresh[1]) Lab[0] = ((float)116.0 * pow(xyz[1], (float) (1.0f / 3.0f)) - 16.0); \n" \
    "        else Lab[1] = 903.3 * xyz[1];                                   \n" \
    "        Lab[1] = 500 * (xyzF[0] - xyzF[1]);                             \n" \
    "        Lab[2] = 200 * (xyzF[1] - xyzF[2]);                             \n" \
    "                                                                        \n" \
    "        for(int ii = 0; ii < 3; ii++) {                                 \n" \
    "            labIm[idx + ii] = Lab[ii];                                  \n" \
    "        }                                                               \n" \
    "    }                                                                   \n" \
    "}                                                                       \n" \
    "\n";

void
rgb2lab(OpenCL &opencl, const Size &size, int stride, Memory &rgb, Memory &lab)
{
    Kernel kernel(opencl, kernelSources, "rgb2lab");

    kernel.setArgument(0, &rgb.getMemory());
    kernel.setArgument(1, &lab.getMemory());
    kernel.setArgument(2, &size.width);
    kernel.setArgument(3, &size.height);
    kernel.setArgument(4, &stride);

    std::vector<size_t> tmpSize(2);
    tmpSize[0] = size.height;
    tmpSize[1] = size.width;
    executeKernel(opencl, kernel, tmpSize);
}
