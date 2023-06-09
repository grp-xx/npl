#include "frame.hpp"
#include "headers.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <iostream>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>


int main(int argc, char* argv[])
{
    if (argc !=2 )
    {
        std::cout << "Usage: " << argv[0] << " <iface>" << std::endl;
        return EXIT_FAILURE;
    }

    // In case of SOCK_DGRAM, adjust protocol in sockt ctor and leave default (all) in sockaddress ctor!
    npl::socket<AF_PACKET, SOCK_RAW> sock(htons(ETH_P_ALL));
    npl::sockaddress<AF_PACKET> device(argv[1]);
    sock.bind(device);
    sock.set_promisc(argv[1]);

    for (;;)
    {
        auto [buf, from] = sock.recvfrom(2000);
        // npl::header<hdr::ipv4> iphdr(buf.data(),buf.size());

        npl::frame ff(buf.data(),buf.size());

        // if (iphdr.protocol() != IPPROTO_ICMP) continue;
        if ( ff.has<hdr::tcp>() )
        {
            auto iphdr = ff.get<hdr::ipv4>();
            std::cout << iphdr.src() << " --> " << iphdr.dst() << "  Options? " <<(iphdr.options().empty() ? "NO" : "SI" ) << std::endl;
        } 

    }

    return EXIT_SUCCESS;
}
