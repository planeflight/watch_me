#include "client.hpp"

#include <CL/cl.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "network.hpp"

std::string load_file(const std::string &file_name) {
    std::ifstream t(file_name);
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string source = buffer.str();
    return source;
}

Client::Client(int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        spdlog::error("Error opening client connection.");
        return;
    }
    spdlog::info("Successfully established client connection.");
    // opencl
    std::string file_contents = load_file("./res/cl/effects.cl");
    gpu_client.program = gpu_client.create_program(file_contents);
    kernel = cl::Kernel(gpu_client.program, "grayscale");
    cl::ImageFormat format{CL_RGBA, CL_UNORM_INT8};
    image = cl::Image2D(
        gpu_client.context, CL_MEM_READ_ONLY, format, WIDTH, HEIGHT);

    prev = cl::Image2D(
        gpu_client.context, CL_MEM_READ_ONLY, format, WIDTH, HEIGHT);
    image_out = cl::Image2D(
        gpu_client.context, CL_MEM_WRITE_ONLY, format, WIDTH, HEIGHT);

    queue = cl::CommandQueue(gpu_client.context, gpu_client.device);
}

void Client::run() {
    spdlog::info("Running client.");
    pollfd client_poll;
    client_poll.fd = sock;
    client_poll.events = POLLIN;

    int size;
    uint32_t size_read = 0;
    Message buffer_msg{.bytes = 0};

    enum class State { SIZE, BUFFER } state = State::SIZE;

    cv::Mat img;
    while (true) {
        if (poll(&client_poll, 1, 33) < 0) {
            spdlog::error("Poll error.");
            continue;
        }

        if (client_poll.revents & POLLIN) {
            if (state == State::SIZE) {
                char *offset = ((char *)&size) + size_read;
                int bytes = recv(sock, offset, sizeof(size) - size_read, 0);
                if (bytes == 0) {
                    spdlog::info("Server shut down.");
                    break;
                } else if (bytes < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                    spdlog::error("Unexpected error.");
                    break;
                }
                // increment number of bytes read
                size_read += bytes;
                // check if we read sizeof(int) = 4 bytes
                if (size_read == sizeof(size)) {
                    size = ntohl(size);
                    state = State::BUFFER;
                    buffer_msg.buffer.resize(size);
                    buffer_msg.bytes = 0;
                }
            } else {
                unsigned char *offset =
                    buffer_msg.buffer.data() + buffer_msg.bytes;
                int bytes = recv(sock, offset, size - buffer_msg.bytes, 0);
                if (bytes == 0) {
                    spdlog::info("Server shut down.");
                    break;
                } else if (bytes < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                    spdlog::error("Unexpected error.");
                    break;
                }
                buffer_msg.bytes += bytes;
                if (buffer_msg.bytes == (size_t)size) {
                    state = State::SIZE;
                    size_read = 0;
                    img = cv::imdecode(buffer_msg.buffer, cv::IMREAD_COLOR);
                    cv::cvtColor(img, img, cv::COLOR_RGB2RGBA);
                    render(img);
                }
            }
        }

        // only for first frames when no image has been sent yet
        if (!img.empty()) cv::imshow("Client", img);

        if (cv::waitKey(1) == 27) break; // ESC to quit
    }
    spdlog::info("Client shutting down.");
    close(sock);
}

void Client::render(cv::Mat &img) {
    cl::NDRange global_size{WIDTH, HEIGHT};
    cl::array<size_t, 3> origin = {0, 0, 0};
    cl::array<size_t, 3> region = {WIDTH, HEIGHT, 1};

    queue.enqueueWriteImage(image,
                            CL_FALSE, // blocking write
                            origin,
                            region,
                            img.step,
                            WIDTH * 4, // row pitch
                            img.data);

    kernel.setArg(0, image);
    kernel.setArg(1, image_out);
    kernel.setArg(2, prev);

    queue.enqueueNDRangeKernel(
        kernel, cl::NullRange, global_size, cl::NullRange);

    // read results
    std::vector<uchar> output(WIDTH * HEIGHT * 4);

    queue.enqueueReadImage(
        image_out, CL_TRUE, origin, region, 0, 0, output.data());

    img = cv::Mat(HEIGHT, WIDTH, CV_8UC4, output.data())
              .clone(); // wrap without copy

    queue.enqueueCopyImage(image, prev, origin, origin, region);
}
