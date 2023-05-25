#include "sockaddress.hpp"
#include "socket.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <sys/_types/_u_short.h>
#include <sys/socket.h>
#include <thread>
#include <tuple>
#include <vector>
#include "json.hpp"

typedef std::tuple< std::string, std::string, unsigned short, unsigned short> flow_t;
typedef std::pair <flow_t, int> tab_entry_t;

void print_and_reset(std::map<flow_t,int>& fmap, std::mutex& mut)
{
    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        mut.lock();
        auto local_map = fmap;
        fmap.clear();
        mut.unlock();

        if ( local_map.empty() ) continue;

        std::vector<tab_entry_t> flow_vec; 

        for (auto &i : local_map) 
        {
            flow_vec.push_back(i);
        }


        // compare(x1,x2) = true se x1 viene prima di x2

        auto compare = [](tab_entry_t x1,tab_entry_t x2) 
                                     {
                                        return (x1.second > x2.second);
                                     };

        std::sort(flow_vec.begin(),flow_vec.end(),compare);

        for (auto i = 0; i < 5; i++)
        {
            std::cout << i+1 << ". " << std::get<0>(flow_vec[i].first) << "  " << std::get<2>(flow_vec[i].first) << " --> "
                                     << std::get<1>(flow_vec[i].first) << "  " << std::get<3>(flow_vec[i].first) << "     "
                                     << flow_vec[i].second << std::endl; 
        }
        std::cout << std::endl << std::endl; 

    }
}


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


    std::thread t(print_and_reset, std::ref(map), std::ref(mut));
    t.detach();


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