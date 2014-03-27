#include "common.hpp"
#include "opencl.hpp"
#include "slicsuperpixels.hpp"
#include "utils.hpp"

/*******************************************************************************
 * Main
 *******************************************************************************/
int
main(int argc, char const *argv[])
{
    std::string inFName = argv[1];
    std::string outFName = argv[2];

    Image img(inFName);
    int superPixelSpacing = 64;
    int nIters = 10;
    float relWeight = 0.5;

    int clusterAssig[img.size().width * img.size().height];
    slicSuperPixels(img, superPixelSpacing, nIters, relWeight, clusterAssig);

    writePgm(outFName, clusterAssig, img.size());

    return EXIT_SUCCESS;
}
