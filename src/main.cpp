#include <iostream>
#include <sys/socket.h>
#include "sockaddress.hpp"
#include "socket.hpp"

int main(int, char**) {
    std::cout << "Hello, world!\n";
    npl::socket<AF_UNIX, SOCK_STREAM> sock;
    // npl::sockaddress<AF_UNIX> srv_addr(...);
    // sock.bind(srv_addr);
    npl::sockaddress<AF_INET> aaa("8.3.4.5",43);
    npl::sockaddress<AF_INET> bbb("",43);

    std::cout<< "Host: " << bbb.host() << " Port: " << aaa.port() <<std::endl;

}
