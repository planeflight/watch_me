#ifndef CLIENT_HPP
#define CLIENT_HPP

#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <opencv2/opencv.hpp>

#include "gpu.hpp"

class Client {
  public:
    Client(int port);
    void run();

  private:
    void render(cv::Mat &img);

    int sock = 0;
    Gpu gpu_client;
    cl::Kernel kernel;
    cl::Image2D image, prev;
    cl::Image2D image_out;
    cl::CommandQueue queue;
};

#endif // CLIENT_HPP
