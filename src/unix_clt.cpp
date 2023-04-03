#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <string>
#include <sys/socket.h>

int main() {

    npl::socket<AF_UNIX, SOCK_STREAM> sock;

    std::string sockname = "/tmp/pippo";

    npl::sockaddress<AF_UNIX> srvAddr(sockname);

    sock.connect(srvAddr);

    std::string msg = "Welcome from NPL class";
    
    sock.write(npl::buffer(msg.begin(),msg.end())); 

    std::cout << "Response from server" << std::endl;

    npl::buffer buf = sock.read(80);

    std::cout << std::string(buf.begin(),buf.end()) << std::endl; 

    sock.close();


    return 0;
}