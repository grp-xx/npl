#include <algorithm>
#include <boost/lockfree/policies.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "packet.hpp"
#include "pcap.hpp"
#include <pcap/pcap.h>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <utility>
#include <boost/lockfree/spsc_queue.hpp>
#include <vector>

typedef std::pair<std::string, int> ip_len_entry;

struct md_producer {
    boost::lockfree::spsc_queue<ip_len_entry, boost::lockfree::capacity<1024>> queue;
    
    void operator()(const pcap_pkthdr* hdr, const u_char* pkt_ptr)
    {
        npl::packet<hdr::ether> frame(pkt_ptr,hdr->caplen);
        auto ip = frame.get<hdr::ipv4>().front();
        ip_len_entry md = std::make_pair(ip.src(), ip.len());
        queue.push(md);
    }
};

void md_consumer(md_producer& md_func) {
    std::unordered_map<std::string,std::pair<int, int>> ip_volume_map;
    typedef std::pair<std::string, std::pair<int,int>> vec_entry_t;

    auto cmp = [](vec_entry_t x, vec_entry_t y) {
        return (x.second.first > y.second.first);
    };

    auto last_ts = std::chrono::steady_clock::now();

    for (;;)
    {
        if (!md_func.queue.empty()) {
            ip_len_entry md;
            md_func.queue.pop(md);
            ip_volume_map[md.first].first++;
            ip_volume_map[md.first].second+=md.second;
        }

        auto now = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::seconds>(now-last_ts).count();
        if (delta > 5) { // 5 seconds or more have passed
            std::vector<vec_entry_t> volume_vec;
            for (auto& x : ip_volume_map) volume_vec.push_back(x);
            std::sort(volume_vec.begin(),volume_vec.end(),cmp);

            std::cout << "----- Top Ten -----" << std::endl;
            std::cout << "IP \t\t pkts \t bytes" << std::endl;
            std::cout << "-------------------" << std::endl;
            for (auto i = 0; i < std::min<int>(10, volume_vec.size()) ; i++ )
                std::cout << volume_vec[i].first << "\t" << volume_vec[i].second.first 
                          << "\t" << volume_vec[i].second.second << std::endl;
            std::cout << "-------------------" << std::endl;
            last_ts = std::chrono::steady_clock::now();
        } 
    }

}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " -i <iface> [filter]" << std::endl;
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-i") != 0) {
        std::cout << "Usage: " << argv[0] << " -i <iface> [filter]" << std::endl;
        return EXIT_FAILURE;
    }

    std:: string dev = argv[2];
    std::string rule = "ip";
    if (argc > 3) 
        rule += " and " + std::string(argv[3]);

    npl::pcap::reader<live> tap(dev);
    tap.open();
    if (!rule.empty()) 
        tap.filter(rule);


    md_producer extract_metadata;

    std::thread t(md_consumer,std::ref(extract_metadata));\
    t.detach();
    tap.loop(extract_metadata);

    return EXIT_SUCCESS;
}