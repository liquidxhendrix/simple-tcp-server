#include <iostream>
#include "ServerSocket.h"



int main() {
    std::cout << "Hello World! From Server \n";

    ServerSocket s(1111);

    s.init();
    s.waitforconnection();

    return 0;
}
