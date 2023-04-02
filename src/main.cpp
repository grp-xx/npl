#include <iostream>
#include <sys/socket.h>
#include "socket.hpp"

int main(int, char**) {
    std::cout << "Hello, world!\n";
    npl::socket<AF_UNIX, SOCK_DGRAM> sock;

}
