#include "headers.hpp"
#include "sockaddress.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <thread>

std::mutex m;

void capture(std::string if_name, int group)
{
    npl::socket<AF_PACKET, SOCK_DGRAM> sock;
    npl::sockaddress<AF_PACKET> device(if_name,ETH_P_IP);
    sock.bind(device);
    sock.set_promisc(if_name);
    sock.set_fanout(group);

    for (;;)
    {
        auto [buf, from] = sock.recvfrom(2000);
        npl::header<hdr::ipv4> iphdr(buf.data(),buf.size());
        m.lock();
        std::cout << "Thread: " << std::this_thread::get_id() << std::endl;
        std::cout << iphdr.src() << " --> " << iphdr.dst() << "  Options? " <<(iphdr.options().empty() ? "NO" : "SI" ) << std::endl;
        m.unlock();
    }


}


int main(int argc, char* argv[])
{
    if (argc !=2 )
    {
        std::cout << "Usage: " << argv[0] << " <iface>" << std::endl;
        return EXIT_FAILURE;
    }

    std::thread t1(capture,argv[1],42), t2(capture,argv[1],42);

    t1.join();
    t2.join();

    return EXIT_SUCCESS;
}