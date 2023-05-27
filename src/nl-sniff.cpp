#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include "socket.hpp"
#include "sockaddress.hpp"
#include "headers.hpp"

int main()
{
    npl:npl::socket<AF_INET, SOCK_RAW> sock(IPPROTO_ICMP);



    for (;;)
    {
        auto [buf, from] = sock.recvfrom(1500);
        // const struct ip *hdr = reinterpret_cast<const struct ip*>(&buf[0]);

        npl::header<hdr::ipv4> hh(&buf[0],buf.size());

        
        // Fix for Mac OS (it is already in H.B.O.)
        #ifdef __APPLE__
            uint16_t ip_hdrlen = hh.c_hdr().ip_hl << 2;
            uint16_t ip_totlen = hh.c_hdr().ip_len + ip_hdrlen;
        #endif

        std::cout << "Version: " << hh.version() << "IP Length: " << ip_totlen << std::endl;
        std::cout << hh.src() << " --> " << hh.dst() << std::endl;
        // std::cout << "From:    " << ntohl( hdr->ip_src.s_addr ) << std::endl;
    }

    return EXIT_SUCCESS;
}