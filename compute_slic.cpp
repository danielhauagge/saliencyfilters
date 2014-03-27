#include "common.hpp"
#include "utils.hpp"
#include "opencl.hpp"
#include "slicsuperpixels.hpp"
#include "colorconversion.hpp"

#include <CMDCore/optparser>

int
main(int argc, char const *argv[])
{
    using namespace cmdc;

    OptionParser::Arguments args;
    OptionParser::Options opts;

    OptionParser optParser(&args, &opts);
    optParser.setNArguments(2, 2);

    optParser.addUsage("<in:img> <out:salicency.pgm>");
    optParser.addDescription("Compute super pixels using the SLIC algorithm by Achanta et. al");
    optParser.addCopyright("2014 by Daniel Hauagge");

    optParser.addFlag("useGPU", "-g", "--gpu", "Use the GPU for computation");

    optParser.addSection("Super Pixel Parameters");
    optParser.addOption("superPixelSpacing", "-s", "P", "--super-pixel-spacing", "Controls spacing between super pixels [default: %default]", "8");
    optParser.addOption("nIters", "-n", "N", "--num-iterations", "How many iterations of K-means to run in super pixel routine [default = %default]", "10");
    optParser.addOption("relWeight", "-r", "W", "--relative-weight", "Relative weight of Color vs Position, higher values favor spatial dimensions [default = %default]", "40");

    optParser.parse(argc, argv);

    std::string inFName = args[0];
    std::string outFName = args[1];

    Image img(inFName);

    int superPixelSpacing = opts["superPixelSpacing"].asInt();
    int nIters = opts["nIters"].asInt();
    float relWeight = opts["relWeight"].asFloat();

    bool useGPU = opts["useGPU"].asBool();
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
