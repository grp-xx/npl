#include <iostream>
#include <sys/socket.h>
#include "sockaddress.hpp"
#include "socket.hpp"

int main(int, char**) {
    std::cout << "Hello, world!\n";
    npl::socket<AF_UNIX, SOCK_STREAM> sock;
    // npl::sockaddress<AF_UNIX> srv_addr(...);
    // sock.bind(srv_addr);

}
