#include "common.hpp"
#include "utils.hpp"
#include "opencl.hpp"
#include "slicsuperpixels.hpp"
#include "colorconversion.hpp"

int
main(int argc, char const *argv[])
{
    std::string inFName = argv[1];
    std::string outFName = argv[2];

    Image img(inFName);
    int superPixelSpacing = 64;
    int nIters = 10;
    float relWeight = 40; // Relative weight of Color vs Position, higher values favor spatial dimensions

    bool useGPU = false;
    OpenCL opencl(useGPU);

    Size spSize = superPixelGridSize(img.size(), superPixelSpacing);

    // 3 dims for color + 2 for x and y
    Memory clusterCenters(opencl, CL_MEM_READ_WRITE, sizeof(float) * spSize.width * spSize.height * 5);
    Memory clusterAssig(opencl, CL_MEM_READ_WRITE, sizeof(int) * img.size().width * img.size().height);

    Memory imgRGB(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img.stride() * img.size().height, img(0, 0));
    Memory imgLab(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img.stride() * img.size().height, img(0, 0));
    rgb2lab(opencl, img.size(), img.stride(), imgRGB, imgLab);

    slicSuperPixels(opencl, img.size(), img.stride(), superPixelSpacing, nIters, relWeight, imgLab, clusterCenters, clusterAssig);

    int clusterAssig_[img.size().width * img.size().height];
    clusterAssig.readBuffer(opencl, clusterAssig_);
    writePgm(outFName, clusterAssig_, img.size());

    return EXIT_SUCCESS;
}
