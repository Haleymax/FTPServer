#include "FTPServer/FTPServer.cpp"

int main() {
    Server server;
    int sock_opt = 1; // SO_REUSEADDR选项

    if (server.init(sock_opt) < 0)
        exit(-1);

    server.serve_client();

    return 0;
}