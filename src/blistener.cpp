#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "sockaddress.hpp"
#include "socket.hpp"
#include "json.hpp"
#include <thread>

using nlohmann::json;

int main()
{
    npl::socket<AF_INET, SOCK_DGRAM> sock;
    sock.broadcast_enable();
    npl::sockaddress<AF_INET> myaddr("",20000);
    sock.bind(myaddr);

    auto [buf,bcaster] = sock.recvfrom(80);

    std::string msg_str(buf.begin(),buf.end());

    json msg = json::parse(msg_str);

    std::cout << "Name:    " << msg.at("name").get<std::string>() << std::endl;
    std::cout << "Address: " << msg.at("ip").get<std::string>() << std::endl;
    std::cout << "Port:    " << msg.at("port").get<int>() << std::endl;

    std::cout << "Data received from: " << bcaster.host() << " :: " << bcaster.port() << std::endl;

    return EXIT_SUCCESS;
}