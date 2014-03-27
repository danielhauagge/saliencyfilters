#include "saliencyfilter.hpp"

static
const char *kernelSources =
    "float eucDist2(float* v1, float* v2, int dim) {                         \n" \
    "   float dist = 0.0;                                                    \n" \
    "   for(int i = 0; i < dim; i++) dist += pow(v1[i] - v2[i], 2.0f);       \n" \
    "   return dist;                                                         \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "float gauss(float d2, float sigma2) {                                   \n" \
    "   return exp((float)(-d2 / (2.0f * sigma2)));                          \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "                                                                        \n" \
    "__kernel void elementUniqueness(                                        \n" \
    "   __global float* clusterCenters,                                      \n" \
    "   __global float* uniqueness,                                          \n" \
    "   const int clustersWidth, const int clustersHeight,                   \n" \
    "   float stdDevUniqueness2)                                             \n" \
    "{                                                                       \n" \
    "   int i = get_global_id(0);                                            \n" \
    "   int j = get_global_id(1);                                            \n" \
    "                                                                        \n" \
    "   if(i < clustersHeight && j < clustersWidth) {                        \n" \
    "      int idx = (i * clustersWidth + j) * 5;                            \n" \
    "      float currColor[3] = {clusterCenters[idx], clusterCenters[idx + 1], clusterCenters[idx + 2]}; \n" \
    "      float currPos[2] = {clusterCenters[idx], clusterCenters[idx + 1]}; \n" \
    "                                                                        \n" \
    "      float uniq = 0.0;                                                 \n" \
    "      float sumWp = 0.0;                                                \n" \
    "      for(int ii = 0; ii < clustersHeight; ii++) {                      \n" \
    "         for(int jj = 0; jj < clustersWidth; jj++) {                    \n" \
    "            int idxOther = (ii * clustersWidth + jj) * 5;               \n" \
    "            float otherColor[3] = {clusterCenters[idxOther], clusterCenters[idxOther + 1], clusterCenters[idxOther + 2]}; \n" \
    "            float otherPos[2] = {clusterCenters[idxOther], clusterCenters[idxOther + 1]}; \n" \
    "            float wp = gauss(eucDist2(currPos, otherPos, 2), stdDevUniqueness2);\n" \
    "            sumWp += wp;                                                \n" \
    "            uniq += wp * eucDist2(currColor, otherColor, 3);            \n" \
    "         }                                                              \n" \
    "      }                                                                 \n" \
    "      uniqueness[i * clustersWidth + j] = uniq / sumWp;                 \n" \
    "   }                                                                    \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "                                                                        \n" \
    "__kernel void elementDistribution(                                      \n" \
    "   __global float* clusterCenters,                                      \n" \
    "   __global float* distribution,                                        \n" \
    "   const int clustersWidth, const int clustersHeight,                   \n" \
    "   float stdDevDistribution2)                                           \n" \
    "{                                                                       \n" \
    "   int i = get_global_id(0);                                            \n" \
    "   int j = get_global_id(1);                                            \n" \
    "                                                                        \n" \
    "   if(i < clustersHeight && j < clustersWidth) {                        \n" \
    "      int idx = (i * clustersWidth + j) * 5;                            \n" \
    "      float currColor[3] = {clusterCenters[idx], clusterCenters[idx + 1], clusterCenters[idx + 2]}; \n" \
    "      float currPos[2] = {clusterCenters[idx], clusterCenters[idx + 1]}; \n" \
    "                                                                        \n" \
    "      float blurP[2] = {0.0, 0.0};                                      \n" \
    "      float blurP2 = 0;                                                 \n" \
    "      float sumWc = 0.0;                                                \n" \
    "      for(int ii = 0; ii < clustersHeight; ii++) {                      \n" \
    "         for(int jj = 0; jj < clustersWidth; jj++) {                    \n" \
    "            int idxOther = (ii * clustersWidth + jj) * 5;               \n" \
    "            float otherColor[3] = {clusterCenters[idxOther], clusterCenters[idxOther + 1], clusterCenters[idxOther + 2]}; \n" \
    "            float otherPos[2] = {clusterCenters[idxOther], clusterCenters[idxOther + 1]}; \n" \
    "                                                                        \n" \
    "            float wc = gauss(eucDist2(currColor, otherColor, 2), stdDevDistribution2); \n" \
    "            sumWc += wc;                                                \n" \
    "            blurP2 += wc * (otherPos[0] * otherPos[0] + otherPos[1] * otherPos[1]);  \n" \
    "            blurP[0] += wc * otherPos[0];                               \n" \
    "            blurP[1] += wc * otherPos[1];                               \n" \
    "         }                                                              \n" \
    "      }                                                                 \n" \
    "                                                                        \n" \
    "      blurP2 /= sumWc;                                                  \n" \
    "      blurP[0] /= sumWc;                                                \n" \
    "      blurP[1] /= sumWc;                                                \n" \
    "                                                                        \n" \
    "      distribution[i * clustersWidth + j] = blurP2 + pow(blurP[0], 2.0f) + pow(blurP[1], 2.0f); \n" \
    "   }                                                                    \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "                                                                        \n" \
    "__kernel void elementSaliency(                                          \n" \
    "   __global const float* uniqueness,                                    \n" \
    "   __global const float* distribution,                                  \n" \
    "   __global float* saliencySP,                                          \n" \
    "   const int clustersWidth, const int clustersHeight,                   \n" \
    "   float k)                                                             \n" \
    "{                                                                       \n" \
    "   int i = get_global_id(0);                                            \n" \
    "   int j = get_global_id(1);                                            \n" \
    "                                                                        \n" \
    "   if(i < clustersHeight && j < clustersWidth) {                        \n" \
    "      int idx = (i * clustersWidth + j);                                \n" \
    "      saliencySP[idx] = uniqueness[idx] * exp((float) -k * distribution[idx]);  \n" \
    "   }                                                                    \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "__kernel void propagateSaliency(                                        \n" \
    "   __global const float* img,                                           \n" \
    "   __global const int* clusterAssig,                                    \n" \
    "   __global const float* saliencySP,                                    \n" \
    "   __global float* saliency,                                            \n" \
    "   const int imWidth, const int imHeight,                               \n" \
    "   const int imStride,                                                  \n" \
    "   const int clustersWidth, const int clustersHeight, int supportSize,  \n" \
    "   float alpha, float beta)                                             \n" \
    "{                                                                       \n" \
    "   int i = get_global_id(0);                                            \n" \
    "   int j = get_global_id(1);                                            \n" \
    "                                                                        \n" \
    "   if(i < imHeight && j < imWidth) {                                    \n" \
    "      int idx = (imStride/4) * i + 3 * j;                               \n" \
    "      float currColor[3] = {img[idx], img[idx + 1], img[idx + 2]};      \n" \
    "      float currPos[2] = {i, j};                                        \n" \
    "                                                                        \n" \
    "      float totalW = 0.0;                                               \n" \
    "      float totalS = 0.0;                                               \n" \
    "                                                                        \n" \
    "      for(int ii = i - supportSize; ii <= i + supportSize; ii++) {      \n" \
    "         if(ii < 0 || ii >= imHeight) continue;                         \n" \
    "         for(int jj = j - supportSize; jj <= j + supportSize; jj++) {   \n" \
    "            if(jj < 0 || jj >= imWidth) continue;                       \n" \
    "                                                                        \n" \
    "            int neighSpIdx = clusterAssig[ii * imWidth + jj];           \n" \
    "            float neighSal = saliencySP[neighSpIdx];                    \n" \
    "                                                                        \n" \
    "            int neighIdx = (imStride/4) * ii + 3 * jj;                  \n" \
    "            float neighColor[3] = {img[neighIdx], img[neighIdx + 1], img[neighIdx + 2]}; \n" \
    "            float neighPos[2] = {ii, jj};                               \n" \
    "                                                                        \n" \
    "            float w = exp((float)-0.5 * ( alpha * eucDist2(currColor, neighColor, 3) + beta * eucDist2(currPos, neighPos, 2))); \n" \
    "            totalW += w;                                                \n" \
    "                                                                        \n" \
    "            totalS += neighSal * w;                                     \n" \
    "         }                                                              \n" \
    "      }                                                                 \n" \
    "      totalS /= totalW;                                                 \n" \
    "      saliency[i * imWidth + j] = totalS;                               \n" \
    "   }                                                                    \n" \
    "}                                                                       \n" \
    "\n";

static
void
elementUniqueness(OpenCL &opencl,
                  const Size &gridSize,
                  Memory &clusterCenters, Memory &uniqueness,
                  float stdDevUniqueness)
{
    Kernel kernel(opencl, kernelSources, "elementUniqueness");

    float stdDevUniqueness2 = stdDevUniqueness * stdDevUniqueness;

    kernel.setArgument(0, &clusterCenters.getMemory());
    kernel.setArgument(1, &uniqueness.getMemory());
    kernel.setArgument(2, &gridSize.width);
    kernel.setArgument(3, &gridSize.height);
    kernel.setArgument(4, &stdDevUniqueness2);

    std::vector<size_t> tmpSize(2);
    tmpSize[0] = gridSize.height;
    tmpSize[1] = gridSize.width;

    TIMER("Computing uniqueness");
    executeKernel(opencl, kernel, tmpSize);
}

static
void
elementDistribution(OpenCL &opencl,
                    const Size &gridSize,
                    Memory &clusterCenters, Memory &distribution,
                    float stdDevDistribution)
{
    Kernel kernel(opencl, kernelSources, "elementDistribution");

    float stdDevDistribution2 = stdDevDistribution * stdDevDistribution;

    kernel.setArgument(0, &clusterCenters.getMemory());
    kernel.setArgument(1, &distribution.getMemory());
    kernel.setArgument(2, &gridSize.width);
    kernel.setArgument(3, &gridSize.height);
    kernel.setArgument(4, &stdDevDistribution2);

    std::vector<size_t> tmpSize(2);
    tmpSize[0] = gridSize.height;
    tmpSize[1] = gridSize.width;

    TIMER("Computing distribution");
    executeKernel(opencl, kernel, tmpSize);
}

static
void
elementSaliency(OpenCL &opencl,
                const Size &gridSize,
                Memory &uniqueness, Memory &distribution,
                Memory &saliency,
                float k)
{
    Kernel kernel(opencl, kernelSources, "elementSaliency");

    kernel.setArgument(0, &uniqueness.getMemory());
    kernel.setArgument(1, &distribution.getMemory());
    kernel.setArgument(2, &saliency.getMemory());
    kernel.setArgument(3, &gridSize.width);
    kernel.setArgument(4, &gridSize.height);
    kernel.setArgument(5, &k);

    std::vector<size_t> tmpSize(2);
    tmpSize[0] = gridSize.height;
    tmpSize[1] = gridSize.width;

    TIMER("Computing saliency");
    executeKernel(opencl, kernel, tmpSize);
}

void
saliencyFiltersSP(OpenCL &opencl,
                  const Size &gridSize,
                  Memory &clusterCenters, Memory &saliencySP,
                  float stdDevUniqueness, float stdDevDistribution, float k)
{
    Memory uniqueness(opencl, CL_MEM_READ_WRITE, sizeof(float) * gridSize.width * gridSize.height);
    Memory distribution(opencl, CL_MEM_READ_WRITE, sizeof(float) * gridSize.width * gridSize.height);

    elementUniqueness(opencl, gridSize, clusterCenters, uniqueness, stdDevUniqueness);
    elementDistribution(opencl, gridSize, clusterCenters, distribution, stdDevDistribution);
    elementSaliency(opencl, gridSize, uniqueness, distribution, saliencySP, k);
}

void
propagateSaliency(OpenCL &opencl, const Size &imgSize, int imgStride,
                  const Size &gridSize,
                  Memory &img, Memory &clusterAssig,
                  Memory &saliencySP, Memory &saliency,
                  float alpha, float beta)
{
    Kernel kernel(opencl, kernelSources, "propagateSaliency");

    int supportSize = 3 * sqrt(1.0 / (2.0 * beta));

    kernel.setArgument( 0, &img.getMemory());
    kernel.setArgument( 1, &clusterAssig.getMemory());
    kernel.setArgument( 2, &saliencySP.getMemory());
    kernel.setArgument( 3, &saliency.getMemory());
    kernel.setArgument( 4, &imgSize.width);
    kernel.setArgument( 5, &imgSize.height);
    kernel.setArgument( 6, &imgStride);
    kernel.setArgument( 7, &gridSize.width);
    kernel.setArgument( 8, &gridSize.height);
    kernel.setArgument( 9, &supportSize);
    kernel.setArgument(10, &alpha);
    kernel.setArgument(11, &beta);

    std::vector<size_t> tmpSize(2);
    tmpSize[0] = imgSize.height;
    tmpSize[1] = imgSize.width;

    TIMER("Propagating saliency");
    executeKernel(opencl, kernel, tmpSize);
}
