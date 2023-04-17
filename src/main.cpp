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
    npl::sockaddress<AF_INET> bbb("172.31.213.105",43);

    std::cout<< "Host: " << bbb.host() << " Port: " << aaa.port() <<std::endl;

    npl::sockaddress<AF_INET> ccc("8.8.8.8",53);

    auto [name, serv] = bbb.nameinfo();
    std::cout<< "Name: " << name << " Service: " << serv <<std::endl;

}
