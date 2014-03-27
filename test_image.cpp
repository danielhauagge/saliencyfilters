#include "image.hpp"

int
main(int argc, char const *argv[])
{
    std::string inFName = argv[1];
    std::string outFName = argv[2];

    Image img(inFName);

    LOG_EXPR(img.stride());
    LOG_EXPR(img.size().width * sizeof(float) * 3);

    for(int i = 0; i < img.size().width * img.size().height * 3; i++) LOG_INFO(img(0, 0)[i] * 255);

    img.writeToFile(outFName);

    return EXIT_SUCCESS;
}
