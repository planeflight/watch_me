#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    // correct endian ordering with bits
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    bind(server_fd, (sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);
    int client_fd = accept(server_fd, nullptr, nullptr);

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) return -1;

    cv::Mat frame;
    std::vector<uchar> buf;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Encode frame as JPEG to shrink size
        cv::imencode(".jpg", frame, buf);

        int size = buf.size();
        send(client_fd, &size, sizeof(size), 0);    // send size first
        send(client_fd, buf.data(), buf.size(), 0); // then data
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
