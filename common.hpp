#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

#include <CMDCore/optparser>
#include <CMDCore/autotimer>
#include <CMDCore/logger>

#define LOG_ERROR(msg) LOG_MSG(cmdc::LOGLEVEL_ERROR, msg)
#define LOG_WARN(msg)  LOG_MSG(cmdc::LOGLEVEL_WARN, msg)
#define LOG_INFO(msg)  LOG_MSG(cmdc::LOGLEVEL_INFO, msg)
#define LOG_DEBUG(msg) LOG_MSG(cmdc::LOGLEVEL_DEBUG, msg)
#define LOG_EXPR(msg)  LOG_DEBUG(#msg << " = " << (msg))
#define LOG_FATAL(err_msg) { LOG_ERROR(err_msg); exit(EXIT_FAILURE); }

#define LOG_MSG(level, msg) { \
        cmdc::Logger::getInstance()->setMessage() << msg; \
        cmdc::Logger::getInstance()->printMessage(level, "saliency", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
    }
#define PROGBAR_START(msg) cmdc::Logger::getInstance()->startProgressBar(cmdc::LOGLEVEL_INFO) << msg
#define PROGBAR_UPDATE(curr, total) cmdc::Logger::getInstance()->updateProgressBar(curr, total, \
        "saliency", __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define TIMER(msg) cmdc::AutoTimer __timer(msg, cmdc::LOGLEVEL_INFO, "saliency", __FILE__, __PRETTY_FUNCTION__, __LINE__)


// #define LOG_EXPR(expr)     std::cout << "DEBUG: " << #expr << " = " << (expr) << std::endl
// #define LOG_INFO(msg)      std::cout << "INFO: " << msg << std::endl
// #define LOG_ERROR(err_msg) std::cout << "ERROR: " << err_msg << std::endl
// #define LOG_FATAL(err_msg) { std::cout << "FATAL: " << err_msg << std::endl; std::cout << std::flush; exit(EXIT_FAILURE); }

class Size
{
public:
    int width, height;
    Size(): width(0), height(0) {}
    Size(int width, int height): width(width), height(height) {}
};

#endif // __COMMON_HPP__
