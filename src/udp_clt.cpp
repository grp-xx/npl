#include "sockaddress.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/socket.h>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << "<udp serv> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string srv_ip = argv[1];
    int port = atoi(argv[2]);

    npl::sockaddress<AF_INET> srv_addr(srv_ip,port);
    npl::socket<AF_INET, SOCK_DGRAM> sock;

    std::string message; // = "Welcome to the NPL lab";

    for(;;)
    {
        std::getline(std::cin,message);
        npl::buffer buf(message.begin(),message.end());
        sock.sendto(buf, srv_addr);
        auto [buff, srv] = sock.recvfrom(512);
        std::cout << std::string(buff.begin(),buff.end()) << std::endl;
    }

    sock.close();




    return EXIT_SUCCESS;
}