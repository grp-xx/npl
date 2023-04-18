#include <cstdint>
#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "utility.hpp"

int main() {

    npl::socket<AF_INET, SOCK_STREAM> sock;
    npl::sockaddress<AF_INET> srvAddr("","npl-service");

    sock.connect(srvAddr);

    std::string input;
    std::cout << "Insert text: ";
    std::getline(std::cin,input);

    npl::buffer buf(input.begin(),input.end());
    npl::addMsgHdr(buf);

    sock.write(buf);

    npl::buffer resp = sock.readn(4);
    auto resp_len = npl::parseMsgHdr(resp);
    npl::buffer resp_text = sock.readn(resp_len);

    std::cout << "Response: ";
    std::cout << std::string(resp_text.begin(),resp_text.end()) << std::endl; 

    sock.close();


    return 0;
}