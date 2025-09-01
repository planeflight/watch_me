#ifndef GPU_HPP
#define GPU_HPP

#include <CL/cl2.hpp>

struct Gpu {
    Gpu();

    cl::Context context;
    cl::Device device;
    cl::Program program;
    cl::Program create_program(const std::string &source);
};

#endif // GPU_HPP
