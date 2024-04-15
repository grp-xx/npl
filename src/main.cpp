#include "sockaddress.hpp"
#include "socket.hpp"
#include <iostream>
#include <sys/socket.h>

int main()
{
    npl::socket<AF_INET, SOCK_STREAM> sock;
    // npl::sockaddress<AF_INET> addr("localhost",1000);
    // npl::sockaddress<AF_INET> addr2("www.google.com",2000);
    // std::cout << "IP: " << addr.host() << "   Port: " << addr.port() << std::endl;
    // std::cout << "IP: " << addr2.host() << "   Port: " << addr2.port() << std::endl;

}