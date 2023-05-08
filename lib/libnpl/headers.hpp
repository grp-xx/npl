#ifndef _HEADERS_HPP_
#define _HEADERS_HPP_

#include <algorithm>
#include <arpa/inet.h>
#include <cstdint>
#include <netinet/ip.h>
#include <sys/types.h>
#include <system_error>
#include <vector>

enum class hdr {ipv4, icmp, tcp, udp};

namespace npl {

template <hdr h>
class header;

template <>
class header <hdr::ipv4>
{
    private:
    struct ip _chdr;
    // Option 
    std::vector<u_char> _options;

    public:
    header() {};

    header(const struct ip* ptr )
    : _chdr(*ptr)
    {
        // Fill option field...
        if (_chdr.ip_hl > 5) 
        {
            auto opt_len = (_chdr.ip_hl << 2) - 20;
            auto opt_begin = reinterpret_cast<const u_char*>(ptr) + 20;
            auto opt_end = opt_begin + opt_len;
            _options.reserve(opt_len);
            std::copy(opt_begin,opt_end,_options.begin());
        }

        // Fix Total length in Mac OS 
        #ifdef __APPLE__
            uint16_t ip_hdrlen = _chdr.ip_hl << 2;
            uint16_t ip_totlen = _chdr.ip_len + ip_hdrlen;
            _chdr.ip_len = htons(ip_totlen);
        #endif
    };

    header(const uint8_t* ptr, ssize_t len)
    {
        if (ptr == nullptr || len < sizeof(struct ip))
        {
            throw std::system_error(errno, std::system_category(), "Packet fragment too short");
        }
        _chdr = *reinterpret_cast<const struct ip*>(ptr);
        // Fill option field...
        if (_chdr.ip_hl > 5) 
        {
            auto opt_len = (_chdr.ip_hl << 2) - 20;
            auto opt_begin = ptr + 20;
            auto opt_end = opt_begin + opt_len;
            _options.reserve(opt_len);
            std::copy(opt_begin,opt_end,_options.begin());
        }
        
        // Fix Total length in Mac OS 
        #ifdef __APPLE__
            uint16_t ip_hdrlen = _chdr.ip_hl << 2;
            uint16_t ip_totlen = _chdr.ip_len + ip_hdrlen;
            _chdr.ip_len = htons(ip_totlen);
        #endif
    }

    header  (header const& rhs) = default;
    header& operator= (header const &) = default;
    header  (header&& rhs) = default;
    header& operator=(header &&) = default;
    ~header() = default; 

    auto 
    c_hdr() const
    {
        return _chdr;
    }

    unsigned short 
    version() const
    {
        return static_cast<unsigned short>(_chdr.ip_v);        
    }

    unsigned short
    protocol() const
    {
        return static_cast<unsigned short>(_chdr.ip_p);        
    }

    unsigned short
    hlen() const
    {
        return static_cast<unsigned short>(_chdr.ip_hl*4);        
    }

    unsigned short
    len() const
    {
        return ntohs( static_cast<unsigned short>(_chdr.ip_len) );        
    }

    std::string
    src() const
    {
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, reinterpret_cast<const void*>(&_chdr.ip_src.s_addr), addr, sizeof(addr));
        return addr;
    }

    std::string
    dst() const
    {
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, reinterpret_cast<const void*>(&_chdr.ip_dst.s_addr), addr, sizeof(addr));
        return addr;
    }

    auto
    options() const
    {
        return _options;
    }


};


}



#endif