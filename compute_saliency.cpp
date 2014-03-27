#include "common.hpp"
#include "utils.hpp"
#include "opencl.hpp"
#include "slicsuperpixels.hpp"
#include "saliencyfilter.hpp"
#include "colorconversion.hpp"

int
main(int argc, char const *argv[])
{
    using namespace cmdc;
    Logger::setLogLevels(cmdc::LOGLEVEL_DEBUG);

    OptionParser::Arguments args;
    OptionParser::Options opts;

    OptionParser optParser(&args, &opts);
    optParser.setNArguments(2, 2);

    optParser.addUsage("<in:img> <out:salicency.pgm>");
    optParser.addDescription("Compute per pixel saliency using algorithm proposed in the paper \"Saliency Filters: Contrast Based Filtering for Salient Region Detection\" by Perazzi et. al.");
    optParser.addCopyright("2014 by Daniel Hauagge");

    optParser.addFlag("useGPU", "-g", "--gpu", "Use the GPU for computation");
    optParser.addFlag("useLab", "-l", "--lab", "Use Lab colorspace");

    optParser.addSection("Super Pixel Parameters");
    optParser.addOption("superPixelSpacing", "-s", "P", "--super-pixel-spacing", "Controls spacing between super pixels [default: %default]", "8");
    optParser.addOption("nIters", "-n", "N", "--num-iterations", "How many iterations of K-means to run in super pixel routine [default = %default]", "10");
    optParser.addOption("relWeight", "-r", "W", "--relative-weight", "Relative weight of Color vs Position, higher values favor spatial dimensions [default = %default]", "40");

    optParser.addSection("Salicency Parameters");
    optParser.addOption("stdDevUniqueness", "-u", "F", "--std-uniqueness", "Standard deviation of uniqueness score [default = %default]", "0.25");
    optParser.addOption("stdDevDistribution", "-d", "F", "--std-distribution", "Standard deviation of distribution score [default = %default]", "2");
    optParser.addOption("k", "-k", "F", "", "Exponent weight in saliency score [default = %default]", "6");

    optParser.addSection("Propagating Saliency Score to Pixels");
    optParser.addOption("alpha", "-a", "F", "--alpha", "Color weight [default = %default]", "0.0333");
    optParser.addOption("beta", "-b", "F", "--beta", "Position weight [default = %default]", "0.0333");

    optParser.parse(argc, argv);

    std::string inFName = args[0];
    std::string outFName = args[1];

    Image img(inFName);
    int superPixelSpacing = opts["superPixelSpacing"].asInt();
    int nIters = opts["nIters"].asInt();
    float relWeight = opts["relWeight"].asFloat(); // Relative weight of Color vs Position, higher values favor spatial dimensions

    float stdDevUniqueness = opts["stdDevUniqueness"].asFloat();
    float stdDevDistribution = opts["stdDevDistribution"].asFloat();
    float k = opts["k"].asFloat();

    float alpha = opts["alpha"].asFloat();
    float beta = opts["beta"].asFloat();

    bool useGPU = opts["useGPU"].asBool();
    OpenCL opencl(useGPU);

    bool useLab = opts["useLab"].asBool();

    Size spSize = superPixelGridSize(img.size(), superPixelSpacing);

    // 3 dims for color + 2 for x and y
    Memory clusterCenters(opencl, CL_MEM_READ_WRITE, sizeof(float) * spSize.width * spSize.height * 5);
    Memory saliencySP(opencl, CL_MEM_READ_WRITE, sizeof(float) * spSize.width * spSize.height);
    Memory saliency(opencl, CL_MEM_READ_WRITE, sizeof(float) * img.size().width * img.size().height);
    Memory clusterAssig(opencl, CL_MEM_READ_WRITE, sizeof(int) * img.size().width * img.size().height);

    Memory imgRGB(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img.stride() * img.size().height, img(0, 0));
    Memory imgLab(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img.stride() * img.size().height, img(0, 0));
    rgb2lab(opencl, img.size(), img.stride(), imgRGB, imgLab);

    slicSuperPixels(opencl, img.size(), img.stride(), superPixelSpacing, nIters, relWeight, (useLab) ? imgLab : imgRGB, clusterCenters, clusterAssig);
    saliencyFiltersSP(opencl, spSize, clusterCenters, saliencySP, stdDevUniqueness, stdDevDistribution, k);
    propagateSaliency(opencl, img.size(), img.stride(), spSize, (useLab) ? imgLab : imgRGB, clusterAssig, saliencySP, saliency, alpha, beta);

    float saliency_[img.size().width * img.size().height];
    saliency.readBuffer(opencl, saliency_);
    writePgm(outFName, saliency_, img.size());

    return EXIT_SUCCESS;
}
