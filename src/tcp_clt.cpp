#include <cstdint>
#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int main() {

    npl::socket<AF_INET, SOCK_STREAM> sock;
    npl::sockaddress<AF_INET> srvAddr("","npl-service");

    sock.connect(srvAddr);

    std::string input;
    std::cout << "Insert text: ";
    std::getline(std::cin,input);

    npl::buffer buf(4);  // sizeof(int) = 4;
    auto msg_len = htonl( input.size() );
    auto pp = reinterpret_cast<uint8_t*>(&msg_len);
    
    std::copy(pp,pp+(sizeof(int)),buf.begin());
    buf.insert(buf.end(),input.begin(),input.end());



    sock.write(buf);

    npl::buffer resp = sock.readn(4);
    msg_len = ntohl ( reinterpret_cast<int&>(resp.front()) ) ; 


    npl::buffer resp_text = sock.readn(msg_len);

    std::cout << "Message length: " << msg_len << std::endl;
    std::cout << std::string(resp_text.begin(),resp_text.end()) << std::endl; 

    sock.close();


    return 0;
}