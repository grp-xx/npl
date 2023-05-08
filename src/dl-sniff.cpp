#include "headers.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <iostream>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <sys/socket.h>


int main(int argc, char* argv[])
{
    if (argc !=2 )
    {
        std::cout << "Usage: " << argv[0] << " <iface>" << std::endl;
        return EXIT_FAILURE;
    }

    npl::socket<AF_PACKET, SOCK_DGRAM> sock;
    npl::sockaddress<AF_PACKET> device(std::string(argv[1]),ETH_P_IP);
    sock.bind(device);
    sock.set_promisc(argv[1]);

    for (;;)
    {
        auto [buf, from] = sock.recvfrom(2000);
        npl::header<hdr::ipv4> iphdr(buf.data(),buf.size());

        std::cout << iphdr.src() << " --> " << iphdr.dst() << "  Options? " <<(iphdr.options().empty() ? "NO" : "SI" ) << std::endl;
    }

    return EXIT_SUCCESS;
}