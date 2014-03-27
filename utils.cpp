#include "utils.hpp"

void
writePgm(const std::string &outFName, float *data, const Size &size)
{
    int tmp[size.width * size.height];

    float maxVal = data[0], minVal = data[0];
    for (int i = 1; i < size.width * size.height; i++) {
        minVal = (minVal > data[i]) ? data[i] : minVal;
        maxVal = (maxVal < data[i]) ? data[i] : maxVal;
    }

    float range = maxVal - minVal;

    for (int i = 0; i < size.width * size.height; i++) {
        tmp[i] = 255.0 * (data[i] - minVal) / range;
    }

    writePgm(outFName, tmp, size);
}

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
