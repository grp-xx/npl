#include "sockaddress.hpp"
#include "socket.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>

int main()
{
    npl::socket<AF_INET, SOCK_DGRAM> sock;
    npl::sockaddress<AF_INET> srv_addr("10.114.62.226",15000);
    // npl::sockaddress<AF_INET> srv_addr(15000);
    sock.bind(srv_addr);

    for(;;)
    {
        auto [buf, clt] = sock.recvfrom(512);
        std::cout << "Received request from IP: " << clt.host() << " - Port: " << clt.port() << std::endl;
        std::transform(buf.begin(),buf.end(),buf.begin(),::toupper);
        sock.sendto(buf, clt); 
    }

    sock.close();
    return EXIT_SUCCESS;
}