#include "pcap.hpp"
#include "pcap_frame.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <iostream>
#include <pcap/pcap.h>
#include <string>
#include <sys/socket.h>
#include "json.hpp"


struct handler 
{
    npl::socket<AF_INET, SOCK_DGRAM> _sock;
    npl::sockaddress<AF_INET>   _collector;

    void operator()(const pcap_pkthdr* phdr, const u_char* ptr)
    {
        auto ff = npl::pcap::frame(phdr, ptr);
        auto iphdr = ff.get<hdr::ipv4>();
        auto tcphdr = ff.get<hdr::tcp>();

        nlohmann::json report;

        report["src_ip"]  = iphdr.src(); 
        report["dst_ip"]  = iphdr.dst();
        report["src_prt"] = tcphdr.srcport(); 
        report["dst_prt"] = tcphdr.dstport(); 
        report["length"]  =  iphdr.len() - iphdr.hlen();

        std::string msg = report.dump();
        _sock.sendto(npl::buffer(msg.begin(),msg.end()), _collector);
    }

};


int main(int argc, char *argv[])
{
    if (argc < 4) 
    {
       std::cout << "Usage: " << argv[0] << " [ -i <device> | -f <filename> ] <collector> " << std::endl; 
       return EXIT_FAILURE; 
    }

    handler h;
    h._collector = npl::sockaddress<AF_INET>(argv[3],12000);

    if (std::string(argv[1]) == "-i")
    {
        npl::pcap::reader<live> probe(argv[2]);
        probe.open();
        probe.filter("tcp"); // probe.filter("ip proto 6");
        probe.loop(h);
    }
    
    if (std::string(argv[1]) == "-f")
    {
        npl::pcap::reader<offline> probe(argv[2]);
        probe.filter("tcp"); // probe.filter("ip proto 6");
        probe.loop(h);
    }
}