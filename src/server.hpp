#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <vector>

class Server {
  public:
    Server(uint32_t port);
    ~Server();

    void client_accept();
    void serve();
    void shutdown();

  private:
    bool check_disconnect(int client, size_t size);

    int sock = 0;
    bool running = true;
    std::vector<int> clients;
};

#endif // SERVER_HPP
