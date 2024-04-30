#include <chrono>
#include <cstdlib>
#include <iostream>
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

    json msg;
    msg["name"] = "News Broadcaster";
    msg["ip"]   = "131.114.178.120";
    msg["port"] = 10010;

    std::string msg_str = msg.dump();
    npl::buffer buf(msg_str.begin(),msg_str.end());

    npl::sockaddress<AF_INET> all("255.255.255.255",20000);

    for(;;)
    {
        sock.sendto(buf, all);
        sleep(5);
        // std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return EXIT_SUCCESS;
}