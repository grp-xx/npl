#include "sockaddress.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <iostream>
#include <sys/_types/_u_short.h>
#include <sys/socket.h>
#include <tuple>
#include "json.hpp"

typedef std::tuple< std::string, std::string, unsigned short, unsigned short> flow_t;
typedef std::pair <flow_t, int> tab_entry_t;


int main(int argc, char* argv[]) {

    std::string srv_ip = "";
    if (argc == 2) 
    {
        srv_ip = argv[1];
    }

    const int srv_prt = 12000;
    
    auto srv_addr = npl::sockaddress<AF_INET>(srv_ip,srv_prt);

    npl::socket<AF_INET, SOCK_DGRAM> sock;
    sock.bind(srv_addr);

    std::map<flow_t,int> map;
    std::mutex mut;


    for (;;)
    {
        auto [buf, from] = sock.recvfrom(360);
        nlohmann::json report = nlohmann::json::parse(std::string(buf.begin(),buf.end()));

        flow_t key = std::make_tuple(report.at("src_ip").get<std::string>(), 
                                     report.at("dst_ip").get<std::string>(),
                                     report.at("src_prt").get<u_short>(),
                                     report.at("dst_prt").get<u_short>());
        


        mut.lock();
        map[key] += report.at("length").get<int>();
        mut.unlock();

    }


    return EXIT_SUCCESS;


}