#include "client.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "network.hpp"

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
    // spdlog::set_level(spdlog::level::debug); // print debug messages
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
                    state = State::BUFFER;
                    buffer_msg.buffer.resize(size);
                    buffer_msg.bytes = 0;
                }
            } else {
                unsigned char *offset =
                    buffer_msg.buffer.data() + buffer_msg.bytes;
                int bytes = recv(sock, offset, size - buffer_msg.bytes, 0);
                // TODO: handle server down, errors, etc
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
                }
            }
        }

        // only for first frames when no image has been sent yet
        if (img.empty()) continue;

        cv::imshow("Client", img);
        if (cv::waitKey(1) == 27) break; // ESC to quit
    }
    spdlog::info("Client shutting down.");
    close(sock);
}
