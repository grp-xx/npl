#ifndef _SOCKADDRRESS_HPP_
#define _SOCKADDRRESS_HPP_

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <system_error>
#include <utility>


namespace npl {

template <int F>
class sockaddress;

template<>
class sockaddress<AF_UNIX> {
    
};

template<>
class sockaddress<AF_INET> {
private:
    socklen_t   _len;
    sockaddr_in _addr;

public:
    explicit sockaddress(in_port_t port = 0)  // Empty socket address
    : _len(sizeof(sockaddr_in))
    {
        memset(&_addr,0,sizeof(sockaddr_in));
        _addr.sin_family = AF_INET;
        _addr.sin_port   = htons(port);
        _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    sockaddress(const std::string& host, const in_port_t& port)
    : _len(sizeof(sockaddr_in))
    {
        struct addrinfo *result; 
        struct addrinfo hints = 
        {
            .ai_flags = 0,
            .ai_family = AF_INET,
            .ai_socktype = 0,
            .ai_protocol = 0 
        };

        if ( (::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result ) != 0 ) )
        {
            throw std::system_error(errno, std::generic_category(), "getaddrinfo");
        } 

        _addr = *(reinterpret_cast<struct sockaddr_in*>(result->ai_addr));
        freeaddrinfo(result);
    }

    sockaddress(const std::string& host, const std::string& service)
    : _len(sizeof(sockaddr_in))
    {
        struct addrinfo *result; 
        struct addrinfo hints = 
        {
            .ai_flags = 0,
            .ai_family = AF_INET,
            .ai_socktype = 0,
            .ai_protocol = 0 
        };

        int errcode;
        if ( (errcode = ::getaddrinfo(host.c_str(), service.c_str(), &hints, &result ) != 0 ) )
        {
            throw std::system_error(errcode, std::generic_category(), "getaddrinfo");
        } 

        _addr = *(reinterpret_cast<struct sockaddr_in*>(result->ai_addr));
        freeaddrinfo(result);
    }    


    sockaddress(const sockaddress&)            = default;
    sockaddress& operator=(const sockaddress&) = default;
    sockaddress(sockaddress&&)                 = default;
    sockaddress& operator=(sockaddress&&)      = default;
    ~sockaddress()                             = default;


    in_port_t 
    port() const
    {
        return ntohs(_addr.sin_port);   
    }

    std::string
    host() const
    {
        char pres[INET_ADDRSTRLEN];

        if ( ( inet_ntop(AF_INET, reinterpret_cast<const void*>(&_addr.sin_addr), pres, sizeof(pres)) ) == nullptr ) 
        {
            throw std::system_error(errno, std::generic_category(), "inet_ntop");
        }
        return pres;
    }

    std::pair<std::string,std::string>
    nameinfo(int flags = 0)
    {
        int errcode;
        char hostname[NI_MAXHOST];
        char service[NI_MAXSERV];
        if ( (errcode = getnameinfo(&this->c_addr(), _len, hostname, sizeof(hostname), service, sizeof(service), flags)) != 0 ) {
            throw std::system_error(errcode, std::generic_category(), "getnameinfo");
        }
        return std::make_pair(hostname, service);        
    }

    unsigned short
    family() const
    {
        return _addr.sin_family;
    }

    socklen_t
    len() const
    {
        return _len;
    }

    socklen_t&
    len()
    {
        return _len;
    }

    const sockaddr&
    c_addr() const
    {
        return reinterpret_cast<const sockaddr&>(_addr);
    }

    sockaddr&
    c_addr()
    {
        return reinterpret_cast<struct sockaddr&>(_addr);
    }

};

template<>
class sockaddress<AF_INET6> {

};



}


#endif