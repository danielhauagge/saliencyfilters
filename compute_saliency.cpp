#include "common.hpp"
#include "opencl.hpp"
#include "slicsuperpixels.hpp"

/*******************************************************************************
 * Main
 *******************************************************************************/
int
main(int argc, char const *argv[])
{
    std::string inFName = argv[1];
    Image img(inFName);
    int superPixelSpacing = 30;
    int nIters = 3;

    slicSuperPixels(img, superPixelSpacing, nIters);

    return EXIT_SUCCESS;
}
