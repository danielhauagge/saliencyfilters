#include "slicsuperpixels.hpp"

static
const char *kernelSources =
    "float dist(const float* v1, __global const float* v2, int superPixelSpacing, float relWeight)\n" \
    "{                                                                       \n" \
    "   float dColor = 0.0;                                                  \n" \
    "   for(int i = 0; i < 3; i++) dColor += pow(v1[i] - v2[i], 2.0f);       \n" \
    "                                                                        \n" \
    "   float dSpat = 0.0;                                                   \n" \
    "   for(int i = 3; i < 5; i++) dSpat += pow(v1[i] - v2[i], 2.0f);        \n" \
    "   dSpat *= pow(relWeight / superPixelSpacing, 2.0f);                   \n" \
    "                                                                        \n" \
    "   return dColor + dSpat;                                               \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "                                                                        \n" \
    "__kernel void initClusterAssig(                                         \n" \
    "   __global int* clusterAssig,                                          \n" \
    "   const int superPixelSpacing,                                         \n" \
    "   const int imWidth, const int imHeight,                               \n" \
    "   const int clustersWidth, const int clustersHeight)                   \n" \
    "{                                                                       \n" \
    "   int i = get_global_id(0);                                            \n" \
    "   int j = get_global_id(1);                                            \n" \
    "                                                                        \n" \
    "   if(i < imHeight && j < imWidth) {                                    \n" \
    "       int ci = i / superPixelSpacing;                                  \n" \
    "       int cj = j / superPixelSpacing;                                  \n" \
    "       clusterAssig[i * imWidth + j] = ci * clustersWidth + cj;         \n" \
    "   }                                                                    \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "                                                                        \n" \
    "__kernel void updateClusterCenters(                                     \n" \
    "   __global float* clusterCenters,                                      \n" \
    "   __global const float* img,                                           \n" \
    "   __global const int* clusterAssig,                                    \n" \
    "   const int superPixelSpacing,                                         \n" \
    "   const int imWidth, const int imHeight,                               \n" \
    "   const int imStride,                                                  \n" \
    "   const int clustersWidth, const int clustersHeight)                   \n" \
    "{                                                                       \n" \
    "   int ci = get_global_id(0);                                           \n" \
    "   int cj = get_global_id(1);                                           \n" \
    "                                                                        \n" \
    "   if((ci < clustersHeight) && (cj < clustersWidth)) {                  \n" \
    "      float centroid[5] = {0.0, 0.0, 0.0, 0.0, 0.0};                    \n" \
    "      float count = 0;                                                  \n" \
    "                                                                        \n" \
    "      int isp = ci * superPixelSpacing;                                 \n" \
    "      int jsp = cj * superPixelSpacing;                                 \n" \
    "      for(int i = isp - superPixelSpacing; i <= isp + superPixelSpacing; i++) {\n" \
    "         if(i < 0 || i >= imHeight) continue;                           \n" \
    "         for(int j = jsp - superPixelSpacing; j <= jsp + superPixelSpacing; j++) {\n" \
    "            if(j < 0 || j >= imWidth) continue;                         \n" \
    "                                                                        \n" \
    "            if(clusterAssig[i * imWidth + j] == (ci * clustersWidth + cj)) {\n" \
    "                unsigned int imPix = (imStride/4) * i + 3 * j;          \n" \
    "                centroid[0] += img[imPix + 0];                          \n" \
    "                centroid[1] += img[imPix + 1];                          \n" \
    "                centroid[2] += img[imPix + 2];                          \n" \
    "                                                                        \n" \
    "                centroid[3] += i;                                       \n" \
    "                centroid[4] += j;                                       \n" \
    "                count++;                                                \n" \
    "            }                                                           \n" \
    "         }                                                              \n" \
    "      }                                                                 \n" \
    "                                                                        \n" \
    "      for(int i = 0, ii = 5 * (ci * clustersWidth + cj); i < 5; i++, ii++)  {\n" \
    "         clusterCenters[ii] = centroid[i] / count;                      \n" \
    "      }                                                                 \n" \
    "   }                                                                    \n" \
    "}                                                                       \n" \
    "                                                                        \n" \
    "                                                                        \n" \
    "__kernel void updateClusterAssignment(                                  \n" \
    "   __global const float* clusterCenters,                                \n" \
    "   __global const float* img,                                           \n" \
    "   __global int* clusterAssig,                                          \n" \
    "   const int superPixelSpacing,                                         \n" \
    "   const int imWidth, const int imHeight,                               \n" \
    "   const int imStride,                                                  \n" \
    "   const int clustersWidth, const int clustersHeight,                   \n" \
    "   float relWeight)"
    "{                                                                       \n" \
    "   int ip = get_global_id(0);                                           \n" \
    "   int jp = get_global_id(1);                                           \n" \
    "                                                                        \n" \
    "   if(ip < imHeight && jp < imWidth) {                                  \n" \
    "      int isp = ip / superPixelSpacing;                                 \n" \
    "      int jsp = jp / superPixelSpacing;                                 \n" \
    "                                                                        \n" \
    "      int pixIdx = ip * (imStride / 4) + jp * 3;                        \n" \
    "      float pixFeat[5] = {img[pixIdx], img[pixIdx + 1], img[pixIdx + 2], ip, jp}; \n" \
    "                                                                        \n" \
    "      int assig = -1;                                                   \n" \
    "      float distAssig = 1e20;                                           \n" \
    "                                                                        \n" \
    "      for(int i = isp - 1; i <= isp + 1; i++) {                         \n" \
    "         if(i < 0 || i >= clustersHeight) continue;                     \n" \
    "         for(int j = jsp - 1; j <= jsp + 1; j++) {                      \n" \
    "            if(j < 0 || j >= clustersWidth) continue;                   \n" \
    "                                                                        \n" \
    "            int clusterIdx = (i * clustersWidth + j) * 5;               \n" \
    "                                                                        \n" \
    "            float clusterDist = dist(pixFeat, &clusterCenters[clusterIdx], superPixelSpacing, relWeight); \n" \
    "            if(clusterDist < distAssig) {                               \n" \
    "               distAssig = clusterDist;                                 \n" \
    "               assig = i * clustersWidth + j;                           \n" \
    "            }                                                           \n" \
    "         }                                                              \n" \
    "      }                                                                 \n" \
    "                                                                        \n" \
    "      //printf(\" distAssig = %f \\n\", distAssig);\n"
    "      clusterAssig[ip * imWidth + jp] = assig;                          \n" \
    "   }                                                                    \n" \
    "}                                                                       \n" \
    "\n";


void
writeCentroidsToFile(float *centroids, int width, int height, const std::string &fname)
{
    Image img(Size(width, height));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < 3; k++) {
                img(j, i)[k] = centroids[5 * (i * width + j) + k];
            }
        }
    }

    img.writeToFile(fname);
}

Size
superPixelGridSize(const Size &imageSize, int superPixelSpacing)
{
    return Size(imageSize.width / superPixelSpacing, imageSize.height / superPixelSpacing);
}

void
slicSuperPixels(OpenCL &opencl,
                const Size &imgSize, int stride,
                int superPixelSpacing, int nIters, float relWeight,
                Memory &img, Memory &clusterCenters, Memory &clusterAssig)
{
    int imNPixels = imgSize.width * imgSize.height;
    // int superPixelSpacing2 = superPixelSpacing * superPixelSpacing;
    // int stride = img.stride();

    assert(stride % sizeof(float) == 0); // inside the kernels I'm assuming that stride is a multiple of sizeof(float)
    // assert(img.nChannels() == 3);

    std::vector<size_t> clustersSize(2);
    Size spGrid = superPixelGridSize(imgSize, superPixelSpacing);
    clustersSize[0] = spGrid.height;
    clustersSize[1] = spGrid.width;

    // 3 dims for color + 2 for x and y
    // Memory clusterCenters(opencl, CL_MEM_READ_WRITE, sizeof(float) * clustersSize[0] * clustersSize[1] * 5);
    // Memory clusterAssig(opencl, CL_MEM_READ_WRITE, sizeof(int) * imNPixels);
    // Memory img(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, stride * imgSize.height, const_cast<float *>(img(0, 0)));

    std::vector<size_t> imsize(2);
    imsize[0] = imgSize.height;
    imsize[1] = imgSize.width;

    // ================================
    // Initialize cluster assig
    // ================================
    Kernel initClusterAssig(opencl, kernelSources, "initClusterAssig");

    initClusterAssig.setArgument(0, &clusterAssig.getMemory());
    initClusterAssig.setArgument(1, &superPixelSpacing);
    initClusterAssig.setArgument(2, &imgSize.width);
    initClusterAssig.setArgument(3, &imgSize.height);
    initClusterAssig.setArgument(4, &clustersSize[1]);
    initClusterAssig.setArgument(5, &clustersSize[0]);

    {
        TIMER("Initializing cluster assignment");
        executeKernel(opencl, initClusterAssig, imsize);
    }

    // ================================
    // Run K-means
    // ================================
    Kernel updateClusterCenters(opencl, kernelSources, "updateClusterCenters");
    updateClusterCenters.setArgument(0, &clusterCenters.getMemory());
    updateClusterCenters.setArgument(1, &img.getMemory());
    updateClusterCenters.setArgument(2, &clusterAssig.getMemory());
    updateClusterCenters.setArgument(3, &superPixelSpacing);
    updateClusterCenters.setArgument(4, &imgSize.width);
    updateClusterCenters.setArgument(5, &imgSize.height);
    updateClusterCenters.setArgument(6, &stride);
    updateClusterCenters.setArgument(7, &clustersSize[1]);
    updateClusterCenters.setArgument(8, &clustersSize[0]);

    Kernel updateClusterAssignment(opencl, kernelSources, "updateClusterAssignment");
    updateClusterAssignment.setArgument(0, &clusterCenters.getMemory());
    updateClusterAssignment.setArgument(1, &img.getMemory());
    updateClusterAssignment.setArgument(2, &clusterAssig.getMemory());
    updateClusterAssignment.setArgument(3, &superPixelSpacing);
    updateClusterAssignment.setArgument(4, &imgSize.width);
    updateClusterAssignment.setArgument(5, &imgSize.height);
    updateClusterAssignment.setArgument(6, &stride);
    updateClusterAssignment.setArgument(7, &clustersSize[1]);
    updateClusterAssignment.setArgument(8, &clustersSize[0]);
    updateClusterAssignment.setArgument(9, &relWeight);

    TIMER("Refining assignment");
    for(int iter = 0; iter < nIters; iter++) {
        executeKernel(opencl, updateClusterCenters, clustersSize);
        executeKernel(opencl, updateClusterAssignment, imsize);
    }
}
