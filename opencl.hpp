#ifndef __SAC_OPENCL_HPP__
#define __SAC_OPENCL_HPP__

#include "common.hpp"

#include <OpenCL/opencl.h>

/*******************************************************************************
 * OpenCL wrappers
 *******************************************************************************/

std::string openCLErrorString(int err);

class OpenCL
{
private:
    cl_device_id _deviceId;              // compute device id
    cl_context _context;                 // compute context
    cl_command_queue _commandsQueue;     // compute command queue

public:
    OpenCL(bool useGPU = false);
    ~OpenCL();

    const cl_context &getContext() const { return _context; }
    const cl_device_id &getDeviceID() const { return _deviceId; }
    const cl_command_queue &getCommandsQueue() const { return _commandsQueue; }
};

class Kernel
{
private:
    cl_program _program;                 // compute program
    cl_kernel _kernel;                   // compute kernel

public:
    Kernel(const OpenCL &opencl, const std::string &source, const std::string &kernelName);
    ~Kernel();

    template<typename T> void setArgument(int position, const T *arg);

    size_t getWorkGroupSize(const OpenCL &opencl);
    const cl_kernel &getKernel() const { return _kernel; }
};

template<typename T>
void
Kernel::setArgument(int position, const T *arg)
{
    int err = 0;
    err = clSetKernelArg(_kernel, position, sizeof(T), arg);
    if(err != CL_SUCCESS) LOG_FATAL("Failed to set kernel arguments! " << err);
}

void executeKernel(const OpenCL &opencl, const Kernel &kernel, size_t domainSize);
void executeKernel(const OpenCL &opencl, const Kernel &kernel, const std::vector<size_t> &domainSize);

class Memory
{
private:
    cl_mem _mem;
    size_t _size;

public:
    Memory(const OpenCL &opencl, cl_mem_flags flags, size_t size, void *host_ptr = NULL);
    ~Memory();

    const cl_mem &getMemory() const { return _mem; }
    cl_mem &getMemory() { return _mem; }

    void readBuffer(const OpenCL &opencl, void *buffer);
};

#endif // __SAC_OPENCL_HPP__
