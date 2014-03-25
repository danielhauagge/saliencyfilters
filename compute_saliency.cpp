#include <iostream>
#include <cmath>

#include <OpenCL/opencl.h>

#define LOG_EXPR(expr) std::cout << "DEBUG: " << #expr << " = " << (expr) << std::endl
#define LOG_INFO(msg) std::cout << "INFO: " << msg << std::endl
#define LOG_ERROR(err_msg) std::cout << "ERROR: " << err_msg << std::endl
#define LOG_FATAL(err_msg) { std::cout << "ERROR: " << err_msg << std::endl; std::cout << std::flush; exit(EXIT_FAILURE); }

/*******************************************************************************
 * OpenCL wrappers
 *******************************************************************************/

std::string
openCLErrorString(int err)
{
    switch (err) {
    case CL_SUCCESS:
        return "Success!";
    case CL_DEVICE_NOT_FOUND:
        return "Device not found.";
    case CL_DEVICE_NOT_AVAILABLE:
        return "Device not available";
    case CL_COMPILER_NOT_AVAILABLE:
        return "Compiler not available";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return "Memory object allocation failure";
    case CL_OUT_OF_RESOURCES:
        return "Out of resources";
    case CL_OUT_OF_HOST_MEMORY:
        return "Out of host memory";
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        return "Profiling information not available";
    case CL_MEM_COPY_OVERLAP:
        return "Memory copy overlap";
    case CL_IMAGE_FORMAT_MISMATCH:
        return "Image format mismatch";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        return "Image format not supported";
    case CL_BUILD_PROGRAM_FAILURE:
        return "Program build failure";
    case CL_MAP_FAILURE:
        return "Map failure";
    case CL_INVALID_VALUE:
        return "Invalid value";
    case CL_INVALID_DEVICE_TYPE:
        return "Invalid device type";
    case CL_INVALID_PLATFORM:
        return "Invalid platform";
    case CL_INVALID_DEVICE:
        return "Invalid device";
    case CL_INVALID_CONTEXT:
        return "Invalid context";
    case CL_INVALID_QUEUE_PROPERTIES:
        return "Invalid queue properties";
    case CL_INVALID_COMMAND_QUEUE:
        return "Invalid command queue";
    case CL_INVALID_HOST_PTR:
        return "Invalid host pointer";
    case CL_INVALID_MEM_OBJECT:
        return "Invalid memory object";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return "Invalid image format descriptor";
    case CL_INVALID_IMAGE_SIZE:
        return "Invalid image size";
    case CL_INVALID_SAMPLER:
        return "Invalid sampler";
    case CL_INVALID_BINARY:
        return "Invalid binary";
    case CL_INVALID_BUILD_OPTIONS:
        return "Invalid build options";
    case CL_INVALID_PROGRAM:
        return "Invalid program";
    case CL_INVALID_PROGRAM_EXECUTABLE:
        return "Invalid program executable";
    case CL_INVALID_KERNEL_NAME:
        return "Invalid kernel name";
    case CL_INVALID_KERNEL_DEFINITION:
        return "Invalid kernel definition";
    case CL_INVALID_KERNEL:
        return "Invalid kernel";
    case CL_INVALID_ARG_INDEX:
        return "Invalid argument index";
    case CL_INVALID_ARG_VALUE:
        return "Invalid argument value";
    case CL_INVALID_ARG_SIZE:
        return "Invalid argument size";
    case CL_INVALID_KERNEL_ARGS:
        return "Invalid kernel arguments";
    case CL_INVALID_WORK_DIMENSION:
        return "Invalid work dimension";
    case CL_INVALID_WORK_GROUP_SIZE:
        return "Invalid work group size";
    case CL_INVALID_WORK_ITEM_SIZE:
        return "Invalid work item size";
    case CL_INVALID_GLOBAL_OFFSET:
        return "Invalid global offset";
    case CL_INVALID_EVENT_WAIT_LIST:
        return "Invalid event wait list";
    case CL_INVALID_EVENT:
        return "Invalid event";
    case CL_INVALID_OPERATION:
        return "Invalid operation";
    case CL_INVALID_GL_OBJECT:
        return "Invalid OpenGL object";
    case CL_INVALID_BUFFER_SIZE:
        return "Invalid buffer size";
    case CL_INVALID_MIP_LEVEL:
        return "Invalid mip-map level";
    default:
        return "Unknown";
    }
}

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

OpenCL::OpenCL(bool useGPU)
{
    // Creating device ID group
    int err = clGetDeviceIDs(NULL, useGPU ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &_deviceId, NULL);
    if(err != CL_SUCCESS) LOG_FATAL("Failed to create a device group!");

    // Create a compute context
    _context = clCreateContext(0, 1, &_deviceId, NULL, NULL, &err);
    if (!_context) LOG_FATAL("Failed to create a compute context!");

    // Create a command queue
    _commandsQueue = clCreateCommandQueue(_context, _deviceId, 0, &err);
    if (!_commandsQueue) LOG_FATAL("Failed to create a command queue!");
}

OpenCL::~OpenCL()
{
    clReleaseCommandQueue(_commandsQueue);
    clReleaseContext(_context);
}

class Kernel
{
private:
    cl_program _program;                 // compute program
    cl_kernel _kernel;                   // compute kernel

public:
    Kernel(const OpenCL &opencl, const std::string &source);
    ~Kernel();

    template<typename T> void setArgument(int position, const T *arg);

    size_t getWorkGroupSize(const OpenCL &opencl);
    const cl_kernel &getKernel() const { return _kernel; }
};

Kernel::Kernel(const OpenCL &opencl, const std::string &source)
{
    int err;

    // Create the compute program from the source buffer
    const char *src = source.c_str();
    _program = clCreateProgramWithSource(opencl.getContext(), 1, (const char **) &src, NULL, &err);
    if (!_program) LOG_FATAL("Failed to create compute program!");

    // Build the program executable
    err = clBuildProgram(_program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        char buffer[2048];

        LOG_ERROR("Failed to build program executable!");
        clGetProgramBuildInfo(_program, opencl.getDeviceID(), CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(EXIT_FAILURE);
    }

    // Create the compute kernel in the program we wish to run
    _kernel = clCreateKernel(_program, "square", &err);
    if (!_kernel || err != CL_SUCCESS) {
        LOG_FATAL("Error: Failed to create compute kernel!");
    }
}

Kernel::~Kernel()
{
    clReleaseProgram(_program);
    clReleaseKernel(_kernel);
}

template<typename T>
void
Kernel::setArgument(int position, const T *arg)
{
    int err = 0;
    err = clSetKernelArg(_kernel, position, sizeof(T), arg);
    if(err != CL_SUCCESS) LOG_FATAL("Failed to set kernel arguments! " << err);
}

size_t
Kernel::getWorkGroupSize(const OpenCL &opencl)
{
    size_t workgroupSize;

    int err = clGetKernelWorkGroupInfo(_kernel, opencl.getDeviceID(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(workgroupSize), &workgroupSize, NULL);
    if (err != CL_SUCCESS) LOG_FATAL("Error: Failed to retrieve kernel work group info! " << err);

    return workgroupSize;
}

void
executeKernel(const OpenCL &opencl, const Kernel &kernel, size_t workgroupSize, size_t domainSize)
{
    LOG_EXPR(workgroupSize);
    LOG_EXPR(domainSize);

    int err = clEnqueueNDRangeKernel(opencl.getCommandsQueue(), kernel.getKernel(), 1, NULL, &domainSize, &workgroupSize, 0, NULL, NULL);
    if (err) LOG_FATAL("Failed to execute kernel: " << openCLErrorString(err));

    clFinish(opencl.getCommandsQueue());
}

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

Memory::Memory(const OpenCL &opencl, cl_mem_flags flags, size_t size, void *host_ptr):
    _size(size)
{
    _mem = clCreateBuffer(opencl.getContext(), flags, size, host_ptr, NULL);
    if (!_mem) LOG_FATAL("Failed to allocate device memory!");
}

Memory::~Memory()
{
    clReleaseMemObject(_mem);
}

void
Memory::readBuffer(const OpenCL &opencl, void *buffer)
{
    size_t offset = 0;

    int err = clEnqueueReadBuffer( opencl.getCommandsQueue(), _mem, CL_TRUE, offset, _size, buffer, 0, NULL, NULL );
    if(err != CL_SUCCESS) LOG_FATAL("Error: Failed to read output array! " << openCLErrorString(err));
}

/*******************************************************************************
 * Kernels
 *******************************************************************************/
const char *KernelSource =
    "__kernel void square(                                                  \n" \
    "   __global float* input,                                              \n" \
    "   __global float* output,                                             \n" \
    "   const unsigned int count)                                           \n" \
    "{                                                                      \n" \
    "   int i = get_global_id(0);                                           \n" \
    "   if(i < count)                                                       \n" \
    "       output[i] = input[i] * input[i];                                \n" \
    "}                                                                      \n" \
    "\n";

/*******************************************************************************
 * Main
 *******************************************************************************/
int
main(int argc, char const *argv[])
{
    OpenCL opencl;
    Kernel kernel(opencl, KernelSource);

    size_t count = 1024;
    float data[count];
    float outputCpu[count];
    for(int i = 0; i < count; i++) data[i] = i;

    Memory input(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * count, data);
    Memory output(opencl, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL);

    kernel.setArgument(0, &input.getMemory());
    kernel.setArgument(1, &output.getMemory());
    kernel.setArgument(2, &count);

    size_t workgroupSize = kernel.getWorkGroupSize(opencl);

    executeKernel(opencl, kernel, workgroupSize, count);

    output.readBuffer(opencl, outputCpu);

    for(int i = 0; i < 10; i++) {
        LOG_INFO(std::pow(data[i], 2.0f) << " == " << outputCpu[i]);
    }

    return EXIT_SUCCESS;
}
