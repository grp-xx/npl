#include <cstdio>
#include <pcap/pcap.h>
#include <iostream>
#include <string>
#include "packet.hpp"
#include "pcap.hpp"

// void elabora(u_char *user, const struct pcap_pkthdr *h,  const u_char *bytes)
// {
//   /* this function is called on per-packet basis */
//     auto cnt = reinterpret_cast<int*>(user);
//     (*cnt)++;
//     npl::packet<hdr::ether> pkt(bytes,h->caplen);
// //    auto all_hdr = pkt.dump();
// //    for (auto x : all) std::cout << PROTOCOL_NAME.at(x.first) << " : " ; 
//     pkt.display();
//     std::cout << std::endl;
// }

struct func_elabora {
  int _cnt = 0;
  void operator()(const struct pcap_pkthdr *h,  const u_char *bytes) 
  {
    ++_cnt;
    npl::packet<hdr::ether> pkt(bytes,h->caplen);
//    auto all_hdr = pkt.dump();
//    for (auto x : all) std::cout << PROTOCOL_NAME.at(x.first) << " : " ; 
    pkt.display();
    std::cout << std::endl;
  }
};


int main(int argc, char* argv[])
{
  if (argc < 3)
  {
      std::cout << "Usage: " << argv[0] << " -i <iface> | -f <filename>" << std::endl;
      return EXIT_FAILURE;
  }  

  std::string flag(argv[1]);
  std::string dev(argv[2]);

  func_elabora elab;

  if (flag == "-f") {
      npl::pcap::reader<offline> tap(dev);
      if (argc == 4) {
        tap.filter(argv[3]);
        tap.print_bpf_program();
        std::cout << "Press any key to start..." << std::endl;
        std::getchar();
      }
      tap.loop(elab);
      }
 	else 
    {
      npl::pcap::reader<live> tap(dev);
      tap.open();
      if (argc == 4) {
        tap.filter(argv[3]);
        tap.print_bpf_program();
        std::cout << "Press any key to start..." << std::endl;
        std::getchar();
      }
      tap.loop(elab);
    }

  std::cout << "Number of processed packets: " << elab._cnt << std::endl;

}