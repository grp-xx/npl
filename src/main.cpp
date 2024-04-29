#include "sockaddress.hpp"
#include "socket.hpp"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "json.hpp"

using nlohmann::json;

struct student {
    unsigned long id;
    std::string name;
};

void to_json(json& j, const student& s)
{
    j = json { {"id", s.id }, {"name", s.name}};   
}

void from_json(const json& j, student& s)
{
    j.at("id").get_to(s.id);
    j.at("name").get_to(s.name);
}

int main()
{
    // npl::socket<AF_INET, SOCK_STREAM> sock;
    // npl::sockaddress<AF_INET> addr("localhost",1000);
    // npl::sockaddress<AF_INET> addr("127.0.0.1",443);
    // npl::sockaddress<AF_INET> addr2("www.google.com",2000);
    // std::cout << "IP: " << addr.host() << "   Port: " << addr.port() << std::endl;
    // std::cout << "IP: " << addr2.host() << "   Port: " << addr2.port() << std::endl;
    // std::cout << "Name: " << addr.nameinfo().first << " - Service: " << addr.nameinfo().second << std::endl;

    student io {.id = 156199, .name = "Gregorio" };

    json jo = io;

    std::string jo_str = jo.dump();

    json rx = json::parse(jo_str);

    std::cout << "Name: " << rx.at("name").get<std::string>() << std::endl;
    std::cout << "ID: " << rx.at("id").get<unsigned long>() << std::endl;

}