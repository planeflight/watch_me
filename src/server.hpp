#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>

class Server {
  public:
    Server(uint32_t port);
    ~Server();

    void serve();

  private:
    int sock = 0;
    bool running = true;
};

#endif // SERVER_HPP
