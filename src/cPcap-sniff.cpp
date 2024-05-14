#include <pcap/pcap.h>
#include <iostream>
#include "packet.hpp"

void elabora(u_char *user, const struct pcap_pkthdr *h,  const u_char *bytes)
{
  /* this function is called on per-packet basis */
    auto cnt = reinterpret_cast<int*>(user);
    (*cnt)++;
    npl::packet<hdr::ether> pkt(bytes,h->caplen);
    auto all = pkt.dump();
    for (auto x : all) std::cout << PROTOCOL_NAME.at(x.first) << " : " ; 
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
 	pcap_t *handle;
 	char errbuf[PCAP_ERRBUF_SIZE];
    const int SNAPLEN = 64;
 
   	// handle = pcap_open_live(argv[1], SNAPLEN, 1, 1000, errbuf);
   	handle = pcap_open_offline(argv[1], errbuf);
   	if (handle == NULL) {
   		std::cerr <<  "Couldn't open device:  " << errbuf << std::endl;
   		return(2);
   	}

    int state = 0;
    pcap_loop(handle, -1, elabora, reinterpret_cast<u_char*>(&state));

    std::cout << "Number of processed packets: " << state << std::endl;

//    for (;;)
//    {
//        struct pcap_pkthdr phdr;
//        auto ptr = pcap_next(handle, &phdr);
//
//        npl::packet<hdr::ether> pkt(ptr,phdr.caplen);
//        auto all = pkt.dump();
//        for (auto x : all) std::cout << PROTOCOL_NAME.at(x.first) << " : " ; 
//        std::cout << std::endl; 
//    }
}