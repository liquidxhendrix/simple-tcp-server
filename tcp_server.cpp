#include <iostream>
#include "ServerSocket.h"



int main() {
    std::cout << "Hello World! From Server \n";

    ServerSocket s(1111);

    s.init();
    while (1)
        s.waitforconnection();

    return 0;
}
