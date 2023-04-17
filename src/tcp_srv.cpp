#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <sys/socket.h>

int main() {

    npl::socket<AF_INET, SOCK_STREAM> sock;

    npl::sockaddress<AF_INET> srvAddr(10010);

    sock.bind(srvAddr);

    sock.listen();

    for(;;) {

        auto [connected, cltAddr] = sock.accept();
     
        npl::buffer buf = connected.read(80);
     
        std::cout << "Message received from: " << cltAddr.host() << std::endl;
     
        connected.write(buf); 
     
    }


    sock.close();
    return 0;
}