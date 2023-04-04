#ifndef _SOCKADDRESS_HPP_
#define _SOCKADDRESS_HPP_

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <system_error>

namespace npl {

    template<int F>
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
        sockaddress(in_port_t port = 0)  // Empty socket address
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

        sockaddress(const in_addr& ip, const in_port_t port) 
        : _len(sizeof(sockaddr_in))
        {
            memset(&_addr,0,sizeof(sockaddr_in));
            _addr.sin_family = AF_INET;
            _addr.sin_port   = htons(port);
            _addr.sin_addr   = ip;
        }


        sockaddress(const std::string& host, const in_port_t port)
        : _len(sizeof(sockaddr_in))
        {
            memset(&_addr,0,sizeof(sockaddr_in));
            _addr.sin_family = AF_INET;
            _addr.sin_port   = htons(port);

            if (host.empty()) {
                _addr.sin_addr.s_addr = htonl(INADDR_ANY);
                return;
            }

            if (inet_pton(AF_INET, host.c_str(), reinterpret_cast<void*>(&_addr.sin_addr.s_addr)) <= 0) {
                throw std::system_error(errno, std::generic_category(),"inet_pton");
            }
        }

        sockaddress(const std::string& host, const std::string& service);


        sockaddress(const sockaddress&)            = default;
        sockaddress& operator=(const sockaddress&) = default;
        sockaddress(sockaddress&&)                 = default;
        sockaddress& operator=(sockaddress&&)      = default;
        ~sockaddress()                             = default;


        const std::string
        host() const
        {
            char buf[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, reinterpret_cast<const void*>(&_addr.sin_addr.s_addr), buf, sizeof(buf)) == nullptr)
            {
                throw std::system_error(errno, std::generic_category(),"inet_ntop");
            }
            return buf;
        }   

        const int 
        port() const
        {
            return ntohs(_addr.sin_port);
        }

        int 
        family() const
        {
            return _addr.sin_family;
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


        std::pair<std::string, std::string> 
        getnameinfo() const;
        
        };



    template<>
    class sockaddress<AF_INET6>;

}


#endif