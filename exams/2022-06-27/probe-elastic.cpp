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
#include "restclient-cpp/restclient.h"


struct handler 
{
    std::string  _url;

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

        RestClient::Response r = RestClient::post(_url, "application/json", report.dump());
        std::cout << "Server response: " << r.code << std::endl;
    }

};


int main(int argc, char *argv[])
{
    if (argc < 4) 
    {
       std::cout << "Usage: " << argv[0] << " [ -i <device> | -f <filename> ] <elastic server> " << std::endl; 
       return EXIT_FAILURE; 
    }

    const std::string index = "nplstats";
    handler h;
    h._url = "http://" + std::string(argv[3]) + ":9200/" + index + "/_doc";

    nlohmann::json conf;
    conf["number_of_shards"] = 1;
    conf["number_of_replicas"] = 0;

    nlohmann::json jindex;
    jindex["settings"] = conf;

    RestClient::Response resp_create_index = RestClient::put("http://" + std::string(argv[3]) + ":9200/" + index,"application/json",jindex.dump());
    
    std::cout << "Response om index create: " << resp_create_index.code << std::endl; 


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