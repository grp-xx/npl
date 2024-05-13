#ifndef _HEADERS_HPP_
#define _HEADERS_HPP_
#include <cstring>
#include <sys/types.h>
#include <system_error>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <unordered_map>
#include <vector>
#include "socket.hpp"


enum class hdr {ether, vlan, arp, ipv4, ipv6, icmp, udp, tcp, unkown};

//constexpr const char * const PROTOCOL_NAME[] = {
//    "Ether",
//    "802.1q",
//    "ARP",
//    "IPv4",
//    "IPv6",
//    "ICMP",
//    "UDP",
//    "TCP"
//};

const std::unordered_map<hdr, const std::string> PROTOCOL_NAME =
    { 
        {hdr::ether, "Ether"},
        {hdr::vlan,  "802.1q"},
        {hdr::arp,   "ARP"},
        {hdr::ipv4,  "IPv4"},
        {hdr::ipv6,  "IPv6"},
        {hdr::icmp,  "ICMP"},
        {hdr::udp,   "UDP"},
        {hdr::tcp,   "TCP"},
    }; 
 
struct vlan_header {
    u_char    vlan_dhost[ETHER_ADDR_LEN];
    u_char    vlan_shost[ETHER_ADDR_LEN];
    u_int16_t vlan_tpid;
    u_int16_t vlan_id;
    u_int16_t ether_type;
};


namespace npl {

    template<hdr h>
    class header;

    template<>
    class header<hdr::ether>
    {
    private:
        const struct ether_header* _ptr;

    public:
        header(const u_int8_t* ptr, ssize_t len)
        {
            if ( (ptr == nullptr) || len < sizeof(ether_header)) 
            {
                throw std::system_error(errno, std::system_category(), "Frame fragment too short");
            }
            _ptr = reinterpret_cast<const struct ether_header*>(ptr);
        }
        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 

        auto
        c_hdr() const
        {
            return *_ptr;
        } 
    
        unsigned short
        ethertype() const{
            return ntohs ( _ptr->ether_type);
        }

        std::string 
        src_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _ptr->ether_shost[0] );
            std::for_each(std::begin(_ptr->ether_shost)+1, std::end(_ptr->ether_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

        std::string 
        dst_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _ptr->ether_dhost[0] );
            std::for_each(std::begin(_ptr->ether_dhost)+1, std::end(_ptr->ether_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }
    };

// Specialize for 802.1Q (VLAN) header

    template<>
    class header<hdr::vlan> {
    private:
        const struct vlan_header* _ptr;

    public:
        header(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(vlan_header)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            _ptr = reinterpret_cast<const vlan_header*>(ptr);
        }

        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 

        auto
        c_hdr() const
        {
            return *_ptr;
        } 
        unsigned short
        ethertype() const
        {
            return ntohs(_ptr->ether_type);
        }       

        unsigned short
        vlan_id() const{
            return ntohs(_ptr->vlan_id & 0x0FFF) ;  // ricontrollare
        }

        unsigned short
        tpid() const
        {
            return ntohs(_ptr->vlan_tpid);
        }

        std::string 
        src_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _ptr->vlan_shost[0] );
            std::for_each(std::begin(_ptr->vlan_shost)+1, std::end(_ptr->vlan_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

        std::string 
        dst_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _ptr->vlan_dhost[0] );
            std::for_each(std::begin(_ptr->vlan_dhost)+1, std::end(_ptr->vlan_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

    };


    // Specialize for ARP header

    template<>
    class header<hdr::arp> {
    private:
        const struct arphdr* _ptr;

    public:
        header()
        : _ptr(nullptr)
        {}

        header(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr==nullptr) || (size < sizeof(arphdr))) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            _ptr = reinterpret_cast<const arphdr*>(ptr);
        }

        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 

        auto
        c_hdr() const
        {
            return *_ptr;
        } 
        unsigned short
        operation() const
        {
            return ntohs( _ptr->ar_op );
        }        

    };



    template<>
    class header<hdr::ipv4> {
    private:
        const struct ip* _ptr;
        uint16_t _len;
    public:
        header(const uint8_t* ptr, uint16_t len)
        : _ptr(reinterpret_cast<const ip*>(ptr)), _len(len)
        {
            if ( (ptr == nullptr) || (len < sizeof(struct ip))) 
            {
                throw std::system_error(errno, std::system_category(), "Packet fragment too short");
            }
            _ptr = reinterpret_cast<const struct ip*>(ptr);
        }

        explicit header(const buffer& buf)
        : _ptr(reinterpret_cast<const ip*>(&buf[0]))
        {
            if ((buf.size() < sizeof(struct ip))) 
            {
                throw std::system_error(errno, std::system_category(), "Packet fragment too short");
            }        
        }

        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 

        auto 
        c_hdr() const
        {
            return *(_ptr);
        }

        unsigned short 
        version() const
        {
            return static_cast<unsigned short>(_ptr->ip_v);        
        }

        unsigned short
        protocol() const
        {
            return static_cast<unsigned short>(_ptr->ip_p);        
        }

        unsigned short
        hlen() const
        {
            return static_cast<unsigned short>(_ptr->ip_hl*4);        
        }

        u_short
        len() const
        {
            return ntohs( static_cast<unsigned short>(_ptr->ip_len) );        
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
            auto hlen = (_ptr->ip_hl << 2);
            if ( (hlen > 20) && (_len > hlen) ){
            // std::cout << "Options detected! Header length: " << (_chdr.ip_hl << 2) <<  std::endl;
               auto opt_begin = reinterpret_cast<const u_char*>(_ptr) + 20;
               auto opt_len   = (_ptr->ip_hl << 2) - 20;
               auto opt_end = opt_begin + opt_len;
               options.reserve(opt_len);
               std::copy(opt_begin,opt_end,options.begin()); // Or copy to ... &_options[0]
            }

    //             Fix Total length in Mac OS --> Works only if packets are capture at network layer via AF_INET, SOCK_RAW
    //             #ifdef __APPLE__
    //             uint16_t ip_hdrlen = _chdr->ip_hl << 2;
    //             uint16_t ip_totlen = _chdr->ip_len + ip_hdrlen;
    //             _chdr->ip_len = htons(ip_totlen);
    //             #endif
                return options;
            }

    };

    template<>
    class header<hdr::udp> {
    private:
        const struct udphdr* _ptr;

    public:
        header(const u_char* ptr, ssize_t size)
        : _ptr(reinterpret_cast<const udphdr*>(ptr))
        {
            if ( (ptr == nullptr) || (size < sizeof(udphdr)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
        }

        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 

        auto
        c_hdr() const
        {
            return *_ptr;
        } 
        unsigned short
        srcport() const{
            return ntohs(_ptr->uh_sport);
        }

        unsigned short
        dstport() const{
            return ntohs(_ptr->uh_dport);
        }

        unsigned short
        length() const{
            return ntohs(_ptr->uh_ulen);
        }

    };


// Specialize for TCP header

    template<>
    class header<hdr::tcp> {
    private:
        const struct tcphdr* _ptr;
        u_int16_t _len;

    public:
        header(const u_char* ptr, ssize_t size) 
        : _ptr(reinterpret_cast<const tcphdr*>(ptr)), _len(size)
        {
            if ( (ptr == nullptr) || (size < sizeof(tcphdr)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
        }

        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 
        unsigned short
        srcport() const{
            return ntohs(_ptr->th_sport);
        }

        auto
        c_hdr() const
        {
            return *_ptr;
        } 
        unsigned short
        dstport() const{
            return ntohs(_ptr->th_dport);
        }

        unsigned short 
        hlen() const
        {
            // Returns the IP header length in bytes
            return static_cast<unsigned short>(_ptr->th_off << 2);
        }

        auto 
        options() const
        {
            buffer options;
            auto hlen = _ptr->th_off << 2;

            if ( (hlen > 20) && (_len > hlen) )
            {
                auto opt_begin = reinterpret_cast<const u_char*>(_ptr) + 20;
                auto opt_len   = (_ptr->th_off << 2) - 20;
                auto opt_end = opt_begin + opt_len;
                options.reserve(opt_len);
                std::copy(opt_begin,opt_end,options.begin()); // Or copy to ... &_options[0]
            }
            return options;
        }

    };


    // Specialize for ICMP header
    
    template<>
    class header<hdr::icmp> {
    private:
        const struct icmp* _ptr; 
    
    public:
        header(const u_char* ptr, ssize_t size) 
        : _ptr(reinterpret_cast<const icmp*>(ptr))
        {
            if ( (ptr == nullptr) || (size < sizeof(icmp)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
        }
    
        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 
        
        unsigned short 
        type() const
        {
            return _ptr->icmp_type;
        }
    
        auto
        c_hdr() const
        {
            return *_ptr;
        } 
        unsigned short 
        code() const
        {
            return _ptr->icmp_code;
        }
    };

}




#endif