#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

#define LOG_EXPR(expr)     std::cout << "DEBUG: " << #expr << " = " << (expr) << std::endl
#define LOG_INFO(msg)      std::cout << "INFO: " << msg << std::endl
#define LOG_ERROR(err_msg) std::cout << "ERROR: " << err_msg << std::endl
#define LOG_FATAL(err_msg) { std::cout << "FATAL: " << err_msg << std::endl; std::cout << std::flush; exit(EXIT_FAILURE); }

class Size
{
public:
    int width, height;
    Size(): width(0), height(0) {}
    Size(int width, int height): width(width), height(height) {}
};

#endif // __COMMON_HPP__
