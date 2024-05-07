#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sys/socket.h>
#include "headers.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include <net/ethernet.h>
#include <thread>

std::mutex mut;

void pkt_display(std::string iface, int grp)
{
    npl::socket<AF_PACKET, SOCK_DGRAM> sock(ntohs(ETHERTYPE_IP));
    npl::sockaddress<AF_PACKET> device(iface);
    sock.bind(device);
    sock.set_promisc(iface);
    sock.set_fanout(grp);
   
    for(;;)
    {
        // auto buf = sock.recv(2000);
        auto [buf, from] = sock.recvfrom(2000);
        npl::header<hdr::ipv4> iph(buf);
        mut.lock();
        std::cout << "--------------" << std::endl;
        std::cout << "Thread: " << std::this_thread::get_id() << std::endl;
        std::cout << "MAC address: " << from.hw_addr() << std::endl;
        std::cout << "HW type: " << from.hw_type() << std::endl;
        std::cout << iph.src() << "  ---->  " << iph.dst()  << std::endl;
        mut.unlock();
    }

}


int main(int argc, char* argv[])
{
    if (argc !=2 )
    {
        std::cout << "Usage: " << argv[0] << " <iface>" << std::endl;
        return EXIT_FAILURE;
    }

    std::thread t1(pkt_display, argv[1], 42), t2(pkt_display, argv[1], 43);

    t1.join();
    t2.join();


    return EXIT_SUCCESS;
}