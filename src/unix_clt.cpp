#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main() {

    //npl::socket<AF_UNIX, SOCK_STREAM> sock;
    npl::socket<AF_UNIX, SOCK_DGRAM> sock;

    std::string sockname = "/tmp/pippo";

    npl::sockaddress<AF_UNIX> srvAddr(sockname);
    npl::sockaddress<AF_UNIX> cltAddr(sockname+std::to_string(getpid()));

    sock.bind(cltAddr);

    // sock.connect(srvAddr);

    std::string msg = "Welcome from NPL class";
    
    // sock.write(npl::buffer(msg.begin(),msg.end())); 
    sock.sendto(npl::buffer(msg.begin(),msg.end()), srvAddr);

    std::cout << "Response from server" << std::endl;

    // npl::buffer buf = sock.read(80);
    auto [buf, server] = sock.recvfrom(80);

    std::cout << std::string(buf.begin(),buf.end()) << std::endl; 

    sock.close();


    return 0;
}