#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <sys/socket.h>

int main() {

    npl::socket<AF_UNIX, SOCK_STREAM> sock;

    std::string sockname = "/tmp/pippo";

    npl::sockaddress<AF_UNIX> srvAddr(sockname);

    sock.bind(srvAddr);

    sock.listen();

    for(;;) {

         auto [connected, cltAddr] = sock.accept();
     
         npl::buffer buf = connected.read(80);
     
         std::cout << "Message received from: " << cltAddr.name() << std::endl;
     
         connected.write(buf); 
     

    }


    sock.close();
    return 0;
}