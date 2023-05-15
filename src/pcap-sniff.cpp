#include "pcap.hpp"
#include "frame.hpp"
#include <cstdlib>

int main()
{
    npl::pcap::reader<live> device("eth0");
    // npl::pcap::reader<live> device;
    // npl::pcap::reader<offline> trace("../Traces/nettare-novlan.pcap");

    for (auto i = 0; i<1000;++i)
    {
        auto [hdr,ptr] = device.next();
        std::cout << "Time:        " << hdr.ts.tv_sec << ":" << hdr.ts.tv_usec << std::endl;
        std::cout << "Packet size: " << hdr.len << std::endl;
        auto ff = npl::frame(ptr,hdr.len);
        if ( ff.has<hdr::ipv4>() )
        {
            auto iphdr = ff.get<hdr::ipv4>();
            std::cout << iphdr.src() << " --> " << iphdr.dst() << std::endl;
        } 
    }

    return EXIT_SUCCESS;
}