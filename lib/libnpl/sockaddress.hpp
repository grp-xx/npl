#ifndef _SOCKADDRESS_HPP_
#define _SOCKADDRESS_HPP_

#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

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
            strncpy(_addr.sun_path,sockname.c_str(),sizeof(_addr.sun_path)-1);
        }

        
        sockaddress(const sockaddress&)            = default;
        sockaddress& operator=(const sockaddress&) = default;
        sockaddress(sockaddress&&)                 = default;
        sockaddress& operator=(sockaddress&&)      = default;
        ~sockaddress()                             = default;


        std::string 
        name() const
        {
            return _addr.sun_path;
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
    class sockaddress<AF_INET>;


    template<>
    class sockaddress<AF_INET6>;

}


#endif