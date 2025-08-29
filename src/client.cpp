#include <arpa/inet.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <vector>

#include "network.hpp"

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        spdlog::error("Error opening client connection.");
        return -1;
    }

    while (true) {
        // TODO: get notified if server shuts down and shut down also
        int size = 0;
        // TODO: change MSG_WAIT_ALL to MSG_DONTWAIT
        int bytes = recv(sock, &size, sizeof(size), MSG_WAITALL);
        if (bytes <= 0) break;

        std::vector<uchar> buf(size);
        recv(sock, buf.data(), size, MSG_WAITALL);

        cv::Mat img = cv::imdecode(buf, cv::IMREAD_COLOR);
        if (img.empty()) break;

        cv::imshow("Client", img);
        if (cv::waitKey(1) == 27) break; // ESC to quit
    }

    close(sock);
    return 0;
}
