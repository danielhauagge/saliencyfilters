#include "image.hpp"

int
main(int argc, char const *argv[])
{
    std::string inFName = argv[1];
    std::string outFName = argv[2];

    Image img(inFName);
    img.writeToFile(outFName);


    return EXIT_SUCCESS;
}
