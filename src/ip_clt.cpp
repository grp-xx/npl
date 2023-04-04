#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main() {

    npl::socket<AF_INET, SOCK_DGRAM> sock;


    npl::sockaddress<AF_INET> srvAddr("127.0.0.1",10000);


    // sock.connect(srvAddr);

    std::string msg = "Welcome from NPL class";
    
    // sock.write(npl::buffer(msg.begin(),msg.end())); 
    sock.sendto(npl::buffer(msg.begin(),msg.end()), srvAddr);


    // npl::buffer buf = sock.read(80);
    auto [buf, server] = sock.recvfrom(80);

    std::cout << "Response from server: " << server.host() << std::endl;
    std::cout << std::string(buf.begin(),buf.end()) << std::endl; 

    sock.close();


    return 0;
}