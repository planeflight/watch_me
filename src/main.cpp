#include "server.hpp"

int main() {
    Server live_stream(8000);
    live_stream.serve();
    return 0;
}
