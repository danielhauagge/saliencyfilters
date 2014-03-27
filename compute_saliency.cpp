#include "common.hpp"
#include "opencl.hpp"
#include "slicsuperpixels.hpp"

void
writePgm(const std::string &outFName, int *data, const Size &size)
{
    std::ofstream f(outFName.c_str());

    int maxVal = data[0];
    for(int i = 1; i < size.width * size.height; i++) {
        maxVal = (maxVal > data[i]) ? maxVal : data[i];
    }

    f << "P2\n"
      << size.width << " " << size.height << "\n"
      << maxVal << "\n";

    for (int i = 0, k = 0; i < size.height; i++) {
        std::string sep = "";
        for (int j = 0; j < size.width; j++, k++) {
            f << sep << data[k];
            sep = " ";
        }
        f << "\n";
    }
}

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

    int clusterAssig[img.size().width * img.size().height];
    slicSuperPixels(img, superPixelSpacing, nIters, clusterAssig);

    writePgm(outFName, clusterAssig, img.size());

    return EXIT_SUCCESS;
}
