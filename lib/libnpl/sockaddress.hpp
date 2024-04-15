#ifndef _SOCKADRRESS_HPP_
#define _SOCKADRRESS_HPP_
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <system_error>
#include <arpa/inet.h>

namespace npl {

template <int F>
class sockaddress;

template<>
class sockaddress<AF_INET> {
private:
    socklen_t    _len;
    sockaddr_in _addr;

public:
    explicit sockaddress(in_port_t port = 0);  //  socket address that uses INADDR_ANY as IP address and port
    sockaddress(const sockaddr_in& addr);
    sockaddress(const in_addr& ip, const in_port_t port);

    sockaddress(const std::string& host, const in_port_t port)
    : _len(sizeof(sockaddr_in))
    {
        struct addrinfo hints, *result;
        hints.ai_family = AF_INET;
        hints.ai_socktype = 0;
        hints.ai_protocol = 0;
        hints.ai_flags = 0;

        int errcode;

        if ( (errcode = ::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result)) != 0 ) {
            throw std::system_error(errno, std::generic_category(), "getaddrinfo");
        }

        _addr = *reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
        freeaddrinfo(result);
    }


    sockaddress(const std::string& host, const std::string& service);
    
    sockaddress(const sockaddress&)            = default;
    sockaddress& operator=(const sockaddress&) = default;
    sockaddress(sockaddress&&)                 = default;
    sockaddress& operator=(sockaddress&&)      = default;
    ~sockaddress()                             = default;

    unsigned short
    port() const;


    std::string
    host() const;    // Return the IP address in the dotted form

    unsigned short
    family() const;

    socklen_t 
    len() const;     // Note: const is part of the signature!

    socklen_t& 
    len();

    sockaddr 
    c_addr() const;                              

    sockaddr& 
    c_addr();

    std::pair<std::string, std::string> 
    getnameinfo() const; 
};


}




#endif