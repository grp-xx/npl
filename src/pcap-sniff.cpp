#include "pcap.hpp"
#include "frame.hpp"
#include <chrono>
#include <cstdlib>
#include <pcap/pcap.h>
#include <thread>

struct handler
{
    int _ipc = 0;
    int _tot = 0;
    std::mutex _mut;

    void operator()(const struct pcap_pkthdr* hdr, const u_char* ptr)
    {
        _mut.lock(); 
        ++_tot;
        _mut.unlock();
        auto ff = npl::frame(ptr,hdr->len);
        if ( ff.has<hdr::ipv4>() )
        {
            _mut.lock();
            ++_ipc;
            _mut.unlock();
        } 
    }

    void print_stats()
    {
        std::cout << "---------------" << std::endl;
        _mut.lock();
        std::cout << "IP packets:      " << _ipc << std::endl;
        std::cout << "Total packets:   " << _tot << std::endl;
        std::cout << "IP packet ratio: " << static_cast<float>(_ipc)/static_cast<float>(_tot) << std::endl;
        _mut.unlock();
    }

};


void monitor(handler& hand)
{
    for (;;) 
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        hand.print_stats();
    }

}


int main()
{
    // npl::pcap::reader<live> device;
    // npl::pcap::reader<live> device("eth0");
    // npl::pcap::reader<offline> trace("../Traces/nettare-novlan.pcap");

//    for (auto i = 0; i<100;++i)
//    {
//        auto [hdr,ptr] = device.next();
//        std::cout << "Time:        " << hdr.ts.tv_sec << ":" << hdr.ts.tv_usec << std::endl;
//        std::cout << "Packet size: " << hdr.len << std::endl;
//        auto ff = npl::frame(ptr,hdr.len);
//        if ( ff.has<hdr::ipv4>() )
//        {
//            auto iphdr = ff.get<hdr::ipv4>();
//            std::cout << iphdr.src() << " --> " << iphdr.dst() << std::endl;
//        } 
//    }

    //npl::pcap::reader<live> device("eth0");
    npl::pcap::reader<live> device("en13");
    device.start();
    // npl::pcap::reader<offline> trace("../Traces/nettare-novlan.pcap");

    handler h;

    std::thread t(monitor,std::ref(h));
    device.loop(h);
    // trace.loop(h);



    

    return EXIT_SUCCESS;
}