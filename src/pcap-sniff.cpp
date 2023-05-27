#include "pcap.hpp"
#include "frame.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <pcap/pcap.h>
#include <sstream>
#include <string>
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
            std::cout << "Protocol:       " << ff.get<hdr::ipv4>().protocol() 
                      << " Packet Length: " << ff.get<hdr::ipv4>().len() 
                      << " From PCAP:     " << hdr->len << std::endl;
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

int main(int argc, char* argv[])
{
    if (argc < 3) 
    {
        std::cout << "Usage: " << argv[0] << " [-i device | -f tracefile] [-r filter | -d dump filter]" << std::endl;
        return EXIT_FAILURE;
    }

    std::stringstream ss;
    bool dumpmode = false;

    if (argc >= 4)
    {
        if (  std::string(argv[3]) == "-r"   || std::string(argv[3]) == "-d") 
        {
            {
                for (int i = 4; i < argc; ++i) 
                {
                    ss << argv[i] << " ";
                }
            }
        }
        if (std::string(argv[3]) == "-d") 
        {
            dumpmode = true;
        }
    }

    if ( std::string(argv[1]) == "-i")
    {
        npl::pcap::reader<live> device(argv[2]);
        device.open();
        device.filter(ss.str());

        if (dumpmode) {
            device.print_bpf_program();
            return EXIT_SUCCESS;
        } 

        handler h;

        std::thread t(monitor,std::ref(h));
        t.detach();
        device.loop(h);
        return EXIT_SUCCESS;
    }

    if ( std::string(argv[1]) == "-f")
    {
        npl::pcap::reader<offline> trace(argv[2]);
        // trace.activate();
        trace.filter(ss.str());
        if (dumpmode) {
            trace.print_bpf_program();
            return EXIT_SUCCESS;
        } 
        
        handler h;

        std::thread t(monitor,std::ref(h));
        t.detach();
        trace.loop(h);
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}