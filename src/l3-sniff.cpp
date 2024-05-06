#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include "headers.hpp"
#include "socket.hpp"
#include "sockaddress.hpp"

int main()
{
    npl::socket<AF_INET, SOCK_RAW> sock(IPPROTO_ICMP);

    for(;;)
    {
        auto [buf, from] = sock.recvfrom(1500);

        if (buf.size() >= sizeof(struct ip)) {
//            const struct ip* iphdr = reinterpret_cast<const ip*>(&buf[0]);
//            npl::sockaddress<AF_INET> src(iphdr->ip_src);
//            npl::sockaddress<AF_INET> dst(iphdr->ip_dst);
//            std::cout << from.host() << " ---> " << dst.host() << std::endl;
            npl::header<hdr::ipv4> iphdr(buf);
            std::cout << iphdr.src() << " ---> " << iphdr.dst() << std::endl;
        }

    }

    return EXIT_SUCCESS;
}