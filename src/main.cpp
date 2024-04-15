#include "sockaddress.hpp"
#include "socket.hpp"
#include <iostream>
#include <sys/socket.h>

int main()
{
    npl::socket<AF_INET, SOCK_STREAM> sock;
    // npl::sockaddress<AF_INET> addr("localhost",1000);
    // std::cout << "IP: " << addr.host() << "   Port: " << addr.port() << std::endl;

}