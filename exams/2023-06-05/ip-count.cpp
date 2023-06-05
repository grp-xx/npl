#include "20230605.hpp"
#include "pcap.hpp"
#include "pcap_frame.hpp"
#include <cstdlib>
#include <pcap/pcap.h>
#include <set>

struct handler
{
    std::set<u_int32_t> _IPset;
    BloomFilter<16000, 3> _bf;
    int _approx = 0;

    void operator()(const struct pcap_pkthdr* hdr, const u_char* ptr)
    {
        auto ff = npl::frame(ptr,hdr->len);
        auto iphdr = ff.get<hdr::ipv4>();
        auto x = iphdr.c_hdr().ip_dst.s_addr;
        _IPset.insert(x);
        if ( !_bf.query(x) )
        {
            ++_approx;
            _bf.insert(x);
        }
    }

};


int main(int argc, char* argv[])
{
    if (argc < 2) 
    {
        std::cout << "Usage: " << argv[0] << " tracefile" << std::endl;
        return EXIT_FAILURE;
    }

    npl::pcap::reader<offline> trace(argv[1]);
    trace.filter("ip");
    // trace.filter("vlan and ip");
    handler IPcounter;
    trace.loop(IPcounter);


    std::cout << "Exact number of distinct IP destination addresses:  " << IPcounter._IPset.size() << std::endl;
    std::cout << "Approx number of distinct IP destination addresses: " << IPcounter._approx << std::endl;
    
    return EXIT_SUCCESS;
}