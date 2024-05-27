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
#include <tuple>
#include <unordered_map>
#include <utility>
#include <boost/lockfree/spsc_queue.hpp>
#include <vector>

typedef std::pair<std::string, int> queue_entry;

struct md_producer {
    boost::lockfree::spsc_queue<queue_entry, boost::lockfree::capacity<1024>> queue;
    
    void operator()(const pcap_pkthdr* hdr, const u_char* pkt_ptr)
    {
        npl::packet<hdr::ether> frame(pkt_ptr,hdr->caplen);
        auto ip = frame.get<hdr::ipv4>().front();
        queue_entry md = std::make_pair(ip.src(), ip.len());
        queue.push(md);
    }
};

void md_consumer(md_producer& md_func) {
    typedef std::tuple<std::string, int,int> vec_entry_t;

    struct {
        std::unordered_map<std::string,std::pair<int, int>> ip_volume_map;
        void operator()(const queue_entry& entry )
        {
            ip_volume_map[entry.first].first++;
            ip_volume_map[entry.first].second+=entry.second;
        }

    } fill_map;

    auto cmp = [](vec_entry_t x, vec_entry_t y) {
        return (get<2>(x) > get<2>(y));
    };

    auto last_ts = std::chrono::steady_clock::now();

    for (;;)
    {
        md_func.queue.consume_all(fill_map);

        auto now = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::seconds>(now-last_ts).count();
        if (delta > 5) { // 5 seconds or more have passed
            std::vector<vec_entry_t> volume_vec;
            for (auto [k,v] : fill_map.ip_volume_map) volume_vec.push_back(std::make_tuple(k,v.first,v.second));
            std::sort(volume_vec.begin(),volume_vec.end(),cmp);

            std::cout << "----- Top Ten -----" << std::endl;
            std::cout << "IP\t\tpkts\tbytes" << std::endl;
            std::cout << "-------------------" << std::endl;
            for (auto i = 0; i < std::min<int>(10, volume_vec.size()) ; i++ )
                std::cout << get<0>(volume_vec[i]) << "\t" << get<1>(volume_vec[i]) 
                          << "\t" << get<2>(volume_vec[i]) << std::endl;
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