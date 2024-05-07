#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sys/socket.h>
#include "headers.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include <net/ethernet.h>


int main()
{
    npl::socket<AF_PACKET, SOCK_DGRAM> sock(ntohs(ETHERTYPE_IP));
    npl::sockaddress<AF_PACKET> device("eth0");
    sock.bind(device);
    sock.set_promisc("eth0");


    for(;;)
    {
        // auto buf = sock.recv(2000);
        auto [buf, from] = sock.recvfrom(2000);
        npl::header<hdr::ipv4> iph(buf);
        std::cout << "--------------" << std::endl;
        std::cout << "MAC address: " << from.hw_addr() << std::endl;
        std::cout << "HW type: " << from.hw_type() << std::endl;
        std::cout << iph.src() << "  ---->  " << iph.dst()  << std::endl;
    }

    return EXIT_SUCCESS;
}