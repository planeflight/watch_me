#include "server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <opencv2/opencv.hpp>
#include <thread>

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
    for (int client : clients) {
        close(client);
    }
    close(sock);
}

void Server::client_accept() {
    while (true) {
        int client_sock = accept(sock, nullptr, nullptr);
        if (client_sock < 0) {
            spdlog::error("Error establishing connection to client.");
            continue;
        }
        // TODO: add client ID/name as first message
        spdlog::info("Established new connection!");

        clients.push_back(client_sock);
    }
}

void Server::serve() {
    if (!running) return;

    std::thread accept_thread([&]() { client_accept(); });
    accept_thread.detach();

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
        for (int i = clients.size() - 1; i >= 0; --i) {
            int client_sock = clients[i];
            if (check_disconnect(client_sock, size)) {
                clients.erase(clients.begin() + i);
                continue;
            }
            // otherwise no disconnect
            send(client_sock, &size, sizeof(size), 0); // send size first
            send(client_sock, buffer.data(), buffer.size(), 0); // then data
        }
    }
    spdlog::info("Closed server.");
}

void Server::shutdown() {
    running = false;
}

bool Server::check_disconnect(int client, size_t size) {
    char buf;
    // MSG_PEEK treats data as unread
    ssize_t bytes_received = recv(client, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
    // socket is closed by the peer
    if (bytes_received == 0) {
        // TODO: id's and names
        spdlog::info("Disconnected from client.");
        return true;
    }
    // no data available because client isn't sending anything
    // obviously, but socket is still open
    if (bytes_received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return false;
    }
    // Other error occurred, potentially indicating a closed
    // connection
    if (bytes_received == -1) {
        spdlog::error("Unexpected error occured.");
        return true;
    }
    return true;
}
