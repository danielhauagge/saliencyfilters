#include "common.hpp"
#include "utils.hpp"
#include "opencl.hpp"
#include "slicsuperpixels.hpp"
#include "saliencyfilter.hpp"
#include "colorconversion.hpp"

int
main(int argc, char const *argv[])
{
    std::string inFName = argv[1];
    std::string outFName = argv[2];

    Image img(inFName);
    int superPixelSpacing = 8;
    int nIters = 10;
    float relWeight = 40; // Relative weight of Color vs Position, higher values favor spatial dimensions

    bool useGPU = false;
    LOG_EXPR(useGPU);
    OpenCL opencl(useGPU);

    Size spSize = superPixelGridSize(img.size(), superPixelSpacing);

    // 3 dims for color + 2 for x and y
    Memory clusterCenters(opencl, CL_MEM_READ_WRITE, sizeof(float) * spSize.width * spSize.height * 5);
    Memory saliencySP(opencl, CL_MEM_READ_WRITE, sizeof(float) * spSize.width * spSize.height);
    Memory saliency(opencl, CL_MEM_READ_WRITE, sizeof(float) * img.size().width * img.size().height);
    Memory clusterAssig(opencl, CL_MEM_READ_WRITE, sizeof(int) * img.size().width * img.size().height);

    Memory imgRGB(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img.stride() * img.size().height, img(0, 0));
    Memory imgLab(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img.stride() * img.size().height, img(0, 0));
    rgb2lab(opencl, img.size(), img.stride(), imgRGB, imgLab);

    LOG_INFO("Computing super pixels");
    slicSuperPixels(opencl, img.size(), img.stride(), superPixelSpacing, nIters, relWeight, imgLab, clusterCenters, clusterAssig);

    LOG_INFO("Computing saliency");
    saliencyFiltersSP(opencl, spSize, clusterCenters, saliencySP);

    LOG_INFO("Propagating salicency measure to pixels");
    propagateSaliency(opencl, img.size(), img.stride(), spSize, imgLab, clusterAssig, saliencySP, saliency);

    // float saliency_[img.size().width * img.size().height];
    // saliency.readBuffer(opencl, saliency_);
    // writePgm(outFName, saliency_, img.size());
    return EXIT_SUCCESS;
}
