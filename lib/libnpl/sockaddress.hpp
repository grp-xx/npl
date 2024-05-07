#ifndef _SOCKADDRRESS_HPP_
#define _SOCKADDRRESS_HPP_

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <system_error>
#include <utility>
#include <sstream>

#ifdef __linux__
    #include <linux/if_packet.h>
    #include <linux/if_ether.h>
#endif


namespace npl {

template <int F>
class sockaddress;

template<>
class sockaddress<AF_UNIX>
{
private:
    socklen_t _len;
    sockaddr_un _addr;

public:
    sockaddress()  // Empty socket address
    : _len(sizeof(sockaddr_un))
    {
        memset(&_addr,0,sizeof(sockaddr_un));
        _addr.sun_family = AF_UNIX;
    }

    explicit sockaddress(const std::string& sockname)
    : _len(sizeof(sockaddr_un))
    {
        memset(&_addr,0,sizeof(sockaddr_un));
        _addr.sun_family = AF_UNIX;
        int offset = 0;
        #ifdef __linux__
        ++offset;
        #endif
        strncpy(_addr.sun_path+offset,sockname.c_str(),sizeof(_addr.sun_path)-1);
    }
    
    sockaddress(const sockaddress&)            = default;
    sockaddress& operator=(const sockaddress&) = default;
    sockaddress(sockaddress&&)                 = default;
    sockaddress& operator=(sockaddress&&)      = default;
    ~sockaddress()                             = default;

    std::string 
    name() const
    {
        int offset = 0;
        #ifdef __linux__
        ++offset;
        #endif
        return _addr.sun_path+offset;
    }
         
    int 
    family() const
    {
        return _addr.sun_family;
    }
           
    socklen_t
    len() const     // Note: const is part of the signature!
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
        return reinterpret_cast<const sockaddr&>(_addr) ;
    }                              
    
    sockaddr& 
    c_addr()
    {
        return reinterpret_cast<sockaddr&>(_addr) ;
    }
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

    sockaddress(const sockaddr_in& addr)
    : _len(sizeof(sockaddr_in))
    {
        memset(&_addr,0,sizeof(sockaddr_in));
        _addr = addr;
    }

    sockaddress(const in_addr& ip, const in_port_t port = 0) 
    : _len(sizeof(sockaddr_in))
    {
        memset(&_addr,0,sizeof(sockaddr_in));
        _addr.sin_family = AF_INET;
        _addr.sin_port   = htons(port);
        _addr.sin_addr   = ip;
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

    std::pair<std::string, std::string> 
    nameinfo(int flags = 0) const
    {
        char hostname[NI_MAXHOST], service[NI_MAXSERV];
        if (::getnameinfo(&this->c_addr(), this->_len, hostname, sizeof(hostname), service, sizeof(service), flags) != 0 ) 
        {
            throw std::system_error(errno, std::generic_category(),"getnameinfo");
        }
        return std::make_pair(hostname, service);
    }
};

template<>
class sockaddress<AF_INET6> {

};

#ifdef __linux__

    template<>
    class sockaddress<AF_PACKET>
    {
    private:
        socklen_t   _len;
        sockaddr_ll _addr;

    public:
        sockaddress()
        : _len(sizeof(sockaddr_ll))
        {
            memset(&_addr, 0, sizeof(sockaddr_ll));
            _addr.sll_family = AF_PACKET;
        }

        sockaddress(int if_index, int protocol = ETH_P_ALL)
        : _len(sizeof(sockaddr_ll))
        {
            memset(&_addr, 0, sizeof(sockaddr_ll));
            _addr.sll_family   = AF_PACKET;
            _addr.sll_ifindex  = if_index;
            _addr.sll_protocol = htons(protocol); 
        }


        sockaddress(const std::string& if_name, int protocol = ETH_P_ALL)
        : _len(sizeof(sockaddr_ll))
        {
            memset(&_addr, 0, sizeof(sockaddr_ll));
            _addr.sll_family   = AF_PACKET;
            _addr.sll_protocol = htons(protocol);
            if ( ( _addr.sll_ifindex = if_nametoindex(if_name.c_str())) == 0 ) 
            {
                throw std::system_error(errno, std::system_category(), "Unkown Interface");
            } 
        }

        sockaddress(const sockaddress&)            = default;
        sockaddress& operator=(const sockaddress&) = default;
        sockaddress(sockaddress&&)                 = default;
        sockaddress& operator=(sockaddress&&)      = default;
        ~sockaddress()                             = default;

        int 
        ifindex() const
        {
            return _addr.sll_ifindex;
        }

        std::string
        ifname() const
        {
            char name[64];

            if ( if_indextoname(_addr.sll_ifindex, name) == nullptr )
            {
                throw std::system_error(errno, std::system_category(), "Interface name error");
            } 

            return _addr.sll_ifindex == 0  ? "Any" : name; 
        }


        std::string
        hw_addr() const{
            std::stringstream ss;
            for (auto i = 0 ; i <= 4; ++i)
            {
               ss << std::hex << static_cast<u_short>(_addr.sll_addr[i]) << ":";
            }
            ss << static_cast<u_short>(_addr.sll_addr[5]);
            return ss.str();
        }

        unsigned short
        hw_len() const{
            return _addr.sll_halen;
        }

        unsigned short
        hw_type() const
        {
            return _addr.sll_hatype;
        }

        unsigned short
        pkt_type() const
        {
            return _addr.sll_pkttype;
        }

        unsigned short
        family() const
        {
            return _addr.sll_family;
        }


        socklen_t
        len() const{
            return _len;
        }

        socklen_t &
        len()
        {
            // return static_cast<socklen_t&>(_len);
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
            return reinterpret_cast<sockaddr&>(_addr);
        }



    };
    #endif


}


#endif