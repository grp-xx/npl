#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <sys/socket.h>

int main() {

    //npl::socket<AF_UNIX, SOCK_STREAM> sock;
    npl::socket<AF_INET, SOCK_DGRAM> sock;

    std::string sockname = "/tmp/pippo";

    npl::sockaddress<AF_INET> srvAddr(10000);

    sock.bind(srvAddr);

    // sock.listen();

    for(;;) {

         // auto [connected, cltAddr] = sock.accept();
     
         // npl::buffer buf = connected.read(80);
         auto [buf, cltAddr] = sock.recvfrom(80);
     
         std::cout << "Message received from: " << cltAddr.host() << std::endl;
     
         // connected.write(buf); 
         sock.sendto(buf, cltAddr);
     
    }


    sock.close();
    return 0;
}