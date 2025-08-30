#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <cstddef>
#include <vector>

constexpr int PORT = 8080;

struct Message {
    size_t bytes;
    std::vector<unsigned char> buffer;
};

#endif // NETWORK_HPP
