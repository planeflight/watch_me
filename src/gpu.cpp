#include "gpu.hpp"

#include <spdlog/spdlog.h>

#include <CL/cl2.hpp>
#include <vector>

Gpu::Gpu() {
    // get all platforms (drivers)
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size() == 0) {
        spdlog::error(" No platforms found. Check OpenCL installation!\n");
        exit(1);
    }

    cl::Platform default_platform = all_platforms[0];
    spdlog::info(fmt::format("Using platform: {}",
                             default_platform.getInfo<CL_PLATFORM_NAME>()));

    // get default device of the default platform
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        spdlog::error(" No devices found. Check OpenCL installation!\n");
        exit(1);
    }
    cl::Device default_device = all_devices[0];
    spdlog::info("Using device: {}", default_device.getInfo<CL_DEVICE_NAME>());

    context = cl::Context({default_device});
    device = default_device;
}

cl::Program Gpu::create_program(const std::string &source) {
    cl::Program::Sources sources;
    sources.push_back(source);

    cl::Program program(context, sources);

    if (program.build({device}) != CL_SUCCESS) {
        spdlog::error(
            fmt::format(" Error building {}",
                        program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)));
    }
    return program;
}
