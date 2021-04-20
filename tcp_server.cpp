#include <iostream>
#include "ServerSocket.h"

int main(int argc, char** argv) {
    std::cout << "Hello World! From Server \n";

    if (argc !=2)
    {
        std::cout << "usage tcp_server <port #>\n";
        exit(0);
    }

    ServerSocket s(atoi(argv[1]));

    s.init();
    s.setEchoModeServer();
    s.waitForConnection();

    return 0;
}
