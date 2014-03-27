#include "common.hpp"
#include "opencl.hpp"

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

    size_t count = 1024;
    float data[count];
    float outputCpu[count];
    for(int i = 0; i < count; i++) data[i] = i;

    Memory input(opencl, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * count, data);
    Memory output(opencl, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL);

    Kernel kernel(opencl, KernelSource, "square");
    kernel.setArgument(0, &input.getMemory());
    kernel.setArgument(1, &output.getMemory());
    kernel.setArgument(2, &count);

    executeKernel(opencl, kernel, count);

    output.readBuffer(opencl, outputCpu);

    for(int i = 0; i < 10; i++) {
        LOG_INFO(std::pow(data[i], 2.0f) << " == " << outputCpu[i]);
    }

    return EXIT_SUCCESS;
}
