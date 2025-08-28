#include "server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <opencv2/opencv.hpp>

#include "spdlog/spdlog.h"

Server::Server(uint32_t port) {
    // SOCK_DGRAM: Datagram sockets are for UDP (different order/duplicate msgs)
    // SOCK_STREAM: TCP sequenced, constant, 2 way stream of data
    // SOCK_RAW: ICMP, not used
    // SOCK_SEQPACKET: record boundaries preserved
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        spdlog::error("Error creating connection.");
        running = false;
        return;
    }
    spdlog::info("Successfully created socket.");

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    // htons: correct endian ordering with bits
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
        spdlog::error("Error binding socket.");
        running = false;
        return;
    }
    spdlog::info("Successfully binded socket.");
    if (listen(sock, 1) < 0) {
        spdlog::error("Error listening to socket.");
        running = false;
        return;
    }
    spdlog::info("Listening on port {}.", port);
}

Server::~Server() {
    close(sock);
}

void Server::serve() {
    if (!running) return;
    int client_sock = accept(sock, nullptr, nullptr);
    if (client_sock < 0) {
        spdlog::error("Error establishing connection to client.");
    }
    cv::VideoCapture capture(0);
    if (!capture.isOpened()) {
        spdlog::error("Failed to open camera");
        return;
    }
    cv::Mat frame;
    std::vector<unsigned char> buffer;
    while (running) {
        capture >> frame;
        if (frame.empty()) break;

        // jpg are more compressed
        cv::imencode(".jpg", frame, buffer);

        int size = buffer.size();
        send(client_sock, &size, sizeof(size), 0);          // send size first
        send(client_sock, buffer.data(), buffer.size(), 0); // then data
    }
    close(client_sock);
}
