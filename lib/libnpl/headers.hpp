#ifndef _HEADERS_HPP_
#define _HEADERS_HPP_

#include "socket.hpp"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <system_error>


enum class hdr {ether, vlan, ipv4, icmp, tcp, udp};

namespace npl {

template <hdr proto> 
class header;

template <>
class header<hdr::ipv4>
{
private:
    const struct ip* _ptr;
    
public:
    explicit header(const buffer& buf)
    : _ptr(reinterpret_cast<const ip*>(&buf[0]))
    {
        if ( ( buf.empty() ) || ( buf.size() < sizeof(struct ip)) ) {
            throw std::system_error(errno, std::system_category(),"Packet fragment too small");
        }
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default;

    auto c_hdr() const
    {
        return *_ptr;
    }

    unsigned short 
    version() const
    {
        return static_cast<unsigned short>(_ptr->ip_v);        
    } 

    unsigned short protocol() const
    {
        return _ptr->ip_p;
    }

    unsigned short hlen() const
    {
        return (_ptr->ip_hl << 2);
    }    

    unsigned short len() const
    {
        return ntohs(_ptr->ip_len);
    }
    std::string
    src() const
    {
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, reinterpret_cast<const void*>(&_ptr->ip_src.s_addr), addr, sizeof(addr));
        return addr;
    }

    std::string
    dst() const
    {
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, reinterpret_cast<const void*>(&_ptr->ip_dst.s_addr), addr, sizeof(addr));
        return addr;
    }

    auto
    options() const
    {
        buffer options;
        if (_ptr->ip_hl > 5) {
        // std::cout << "Options detected! Header length: " << (_chdr.ip_hl << 2) <<  std::endl;
           auto opt_begin = reinterpret_cast<const u_char*>(_ptr) + 20;
           auto opt_len   = (_ptr->ip_hl << 2) - 20;
           auto opt_end = opt_begin + opt_len;
           options.reserve(opt_len);
           std::copy(opt_begin,opt_end,options.begin()); // Or copy to ... &_options[0]
    }
        
//             Fix Total length in Mac OS --> Works only if packets are capture at network layer via AF_INET, SOCK_RAW
//             #ifdef __APPLE__
//             uint16_t ip_hdrlen = _ptr->ip_hl << 2;
//             uint16_t ip_totlen = _ptr->ip_len + ip_hdrlen;
//             _chdr->ip_len = htons(ip_totlen);
//             #endif
            return options;
        }


};


}




#endif