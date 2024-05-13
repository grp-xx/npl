#include <cstdlib>
#include <iostream>
#include <linux/if_ether.h>
#include <ostream>
#include <sys/socket.h>
#include "headers.hpp"
#include "packet.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include <net/ethernet.h>


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <iface>" << std::endl;
        return EXIT_FAILURE;
    }


    npl::socket<AF_PACKET, SOCK_RAW> sock(ntohs(ETH_P_ALL));
    npl::sockaddress<AF_PACKET> device(argv[1]);
    sock.bind(device);
    sock.set_promisc(argv[1]);


    for(;;)
    {
        // auto buf = sock.recv(2000);
        // auto [buf, from] = sock.recvfrom(2000);
        // npl::header<hdr::ipv4> iph(buf);
        auto buf = sock.recv(2000);
        npl::packet<hdr::ether> p(buf.data(),buf.size());
        std::cout << "--------------" << std::endl;
        // std::cout << "MAC address: " << from.hw_addr() << std::endl;
        // std::cout << "HW type: " << from.hw_type() << std::endl;
        // std::cout << iph.src() << "  ---->  " << iph.dst()  << std::endl;
        auto all = p.dump();
        for (auto x : all) std::cout << PROTOCOL_NAME.at(x.first) << " : " ; 
        std::cout<< std::endl; 
//        std::cout << p.get<hdr::ipv4>()[0].src() << "--->" << p.get<hdr::ipv4>()[0].dst() << std::endl;
//        std::cout << "Options: " << std::string(p.get<hdr::ipv4>()[0].options().begin(), p.get<hdr::ipv4>()[0].options().end()) << std::endl;
    }

    return EXIT_SUCCESS;
}