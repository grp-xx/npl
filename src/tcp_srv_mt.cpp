#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <thread>

#include "socket.hpp"
#include "sockaddress.hpp"


void capital_echo(npl::socket<AF_INET, SOCK_STREAM>&& connected, const npl::sockaddress<AF_INET>& client)
{
    std::cout << "Remote client " << client.host() << " : " << client.port() << " connected" << std::endl;
    for(;;)
    {
        auto buf = connected.read(256);
        if (buf.empty()) 
            break;
        
        std::transform(buf.begin(),buf.end(),buf.begin(),::toupper);
        connected.write(buf);
    }
    connected.close();
    std::cout << "Remote client " << client.host() << " : " << client.port() << " disconnected" << std::endl;
}


int main()
{
    npl::socket<AF_INET, SOCK_STREAM> sock;
    // npl::sockaddress<AF_INET> srv_addr("10.114.62.226",15000);
    npl::sockaddress<AF_INET> srv_addr(16000);
    sock.bind(srv_addr);
    sock.listen();

    for(;;)
    {
        auto [conn, clt] = sock.accept();
        std::thread t(capital_echo, std::move(conn), clt);
        t.detach();
    }
    sock.close();
    return EXIT_SUCCESS;
}