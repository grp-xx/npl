#include <pcap/pcap.h>
#include <iostream>
#include <string>
#include "packet.hpp"

void elabora(u_char *user, const struct pcap_pkthdr *h,  const u_char *bytes)
{
  /* this function is called on per-packet basis */
    auto cnt = reinterpret_cast<int*>(user);
    (*cnt)++;
    npl::packet<hdr::ether> pkt(bytes,h->caplen);
//    auto all_hdr = pkt.dump();
//    for (auto x : all) std::cout << PROTOCOL_NAME.at(x.first) << " : " ; 
    pkt.display();
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
 	pcap_t *handle;
 	char errbuf[PCAP_ERRBUF_SIZE];
  const int SNAPLEN = 64;

  if (argc != 3)
  {
      std::cout << "Usage: " << argv[0] << " -i <iface> | -f <filename>" << std::endl;
      return EXIT_FAILURE;
  }  

  std::string flag(argv[1]);
  std::string dev(argv[2]);

  if (flag == "-f") {
 	    handle = pcap_open_offline(dev.c_str(), errbuf);
      if (handle == NULL) {
 		  std::cerr <<  "Couldn't open file:  " << errbuf << std::endl;
 		  return(2);
      }
 	} else {
   	handle = pcap_open_live(dev.c_str(), SNAPLEN, 1, 1000, errbuf);
   	if (handle == NULL) {
   		std::cerr <<  "Couldn't open device:  " << errbuf << std::endl;
   		return(2);
    }
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