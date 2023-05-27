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
#include <pcap/pcap.h>
#include <vector>


enum class hdr {pcap, ether, vlan, arp, ipv4, ipv6, icmp, tcp, udp};

    struct vlan_header {
        u_char    vlan_dhost[ETHER_ADDR_LEN];
        u_char    vlan_shost[ETHER_ADDR_LEN];
        u_int16_t vlan_tpid;
        u_int16_t vlan_id;
        u_int16_t ether_type;
    };


namespace npl {

    template <hdr value>
    class header;

    template<>
    class header<hdr::pcap>
    {
    private:
        struct pcap_pkthdr _chdr;
    
    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct pcap_pkthdr));
        }
    
        explicit header(const pcap_pkthdr* ptr) 
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Can't parse PCAP header");
            }
            _chdr = *ptr;
        }

        header(const u_char* ptr, ssize_t len)
        {
            if ( (ptr == nullptr) || len < sizeof(pcap_pkthdr)) 
            {
                throw std::system_error(errno, std::system_category(), "PCAP snaplen too short");
            }
            _chdr = *reinterpret_cast<const struct pcap_pkthdr*>(ptr);
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

        auto
        timestamp() const
        {
            return _chdr.ts;
        }

        auto 
        caplen() const
        {
            return _chdr.caplen;
        }        

        auto
        len() const
        {
            return _chdr.len;
        }

    };

    template<>
    class header<hdr::ether>
    {
    private:
        struct ether_header _chdr;

    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct ether_header));
        }
        explicit header(const ether_header* ptr) 
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Can't parse Ethernet header");
            }
            _chdr = *ptr;
        }
        
        header(const u_char* ptr, ssize_t len)
        {
            if ( (ptr == nullptr) || len < sizeof(ether_header)) 
            {
                throw std::system_error(errno, std::system_category(), "Frame fragment too short");
            }
            _chdr = *reinterpret_cast<const struct ether_header*>(ptr);
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
        ethertype() const{
            return ntohs ( _chdr.ether_type );
        }

        std::string 
        src_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _chdr.ether_shost[0] );
            std::for_each(std::begin(_chdr.ether_shost)+1, std::end(_chdr.ether_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

        std::string 
        dst_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _chdr.ether_dhost[0] );
            std::for_each(std::begin(_chdr.ether_dhost)+1, std::end(_chdr.ether_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }
    };

// Specialize for 802.1Q (VLAN) header

    template<>
    class header<hdr::vlan> {
    private:
        struct vlan_header _chdr = {0};

    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct vlan_header));
        }
        
        explicit header(const vlan_header* ptr) 
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Can't parse Ethernet header");
            }
            _chdr = *ptr;
        }

        header(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(vlan_header)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            _chdr = *reinterpret_cast<const vlan_header*>(ptr);
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
        ethertype() const
        {
            return ntohs(_chdr.ether_type);
        }       

        unsigned short
        vlan_id() const{
            return ntohs(_chdr.vlan_id & 0x0FFF) ;  // ricontrollare
        }

        unsigned short
        tpid() const
        {
            return ntohs(_chdr.vlan_tpid);
        }

        std::string 
        src_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _chdr.vlan_shost[0] );
            std::for_each(std::begin(_chdr.vlan_shost)+1, std::end(_chdr.vlan_shost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

        std::string 
        dst_mac() const
        {
            std::stringstream ss;
            ss << std::hex << static_cast<uint16_t>( _chdr.vlan_dhost[0] );
            std::for_each(std::begin(_chdr.vlan_dhost)+1, std::end(_chdr.vlan_dhost), [&ss](uint8_t x) { ss << ":" << std::hex << static_cast<uint16_t>(x); } ) ;
            return ss.str();
        }

    };


    // Specialize for ARP header

    template<>
    class header<hdr::arp> {
    private:
        struct arphdr _chdr;

    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct arphdr));
        }

        explicit header(const arphdr* ptr) 
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Can't parse ARP protocol");
            }
            _chdr = *ptr;
        }

        header(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr==nullptr) || (size < sizeof(arphdr))) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            _chdr = *reinterpret_cast<const arphdr*>(ptr);
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
        operation() const
        {
            return ntohs( _chdr.ar_op );
        }        

    };


    template <>
    class header<hdr::ipv4>
    {
    private:
        struct ip _chdr;
        std::vector<u_char> _options;
        
    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct ip));
        }
        explicit header (const ip* ptr)
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Packet fragment too short");
            }
            _chdr = *ptr;
            if (_chdr.ip_hl > 5) {
                auto opt_begin = reinterpret_cast<const u_char*>(ptr) + 20;
                auto opt_len   = (_chdr.ip_hl << 2) - 20;
                auto opt_end = opt_begin + opt_len;
                _options.reserve(opt_len);
                std::copy(opt_begin,opt_end,_options.begin()); // Or copy to ... &_options[0]
            }
            // Fix Total length in Mac OS --> Works only if packets are capture at network layer via AF_INET, SOCK_RAW
//            #ifdef __APPLE__
//                uint16_t ip_hdrlen = _chdr.ip_hl << 2;
//                uint16_t ip_totlen = _chdr.ip_len + ip_hdrlen;
//                _chdr.ip_len = htons(ip_totlen);
//            #endif
        }

        header (const uint8_t* ptr, ssize_t size)
        {
            if ( (ptr == nullptr) || size < sizeof(ip)) 
            {
                throw std::system_error(errno, std::system_category(), "Packet fragment too short");
            }
            _chdr = *reinterpret_cast<const struct ip*>(ptr);
            
            if (_chdr.ip_hl > 5) {
                std::cout << "Options detected! Header length: " << (_chdr.ip_hl << 2) <<  std::endl;
                auto opt_begin = reinterpret_cast<const u_char*>(ptr) + 20;
                auto opt_len   = (_chdr.ip_hl << 2) - 20;
                auto opt_end = opt_begin + opt_len;
                _options.reserve(opt_len);
                std::copy(opt_begin,opt_end,_options.begin()); // Or copy to ... &_options[0]
            }
            
            // Fix Total length in Mac OS --> Works only if packets are capture at network layer via AF_INET, SOCK_RAW
//            #ifdef __APPLE__
//                uint16_t ip_hdrlen = _chdr.ip_hl << 2;
//                uint16_t ip_totlen = _chdr.ip_len + ip_hdrlen;
//                _chdr.ip_len = htons(ip_totlen);
//            #endif
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

        u_short
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

    // Specialize for UDP header

    template<>
    class header<hdr::udp> {
    private:
        struct udphdr _chdr = {0};

    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct udphdr));
        }
        explicit header(const udphdr* ptr) 
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Packet fragment too short");
            }
        }

        header(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(udphdr)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            _chdr = *reinterpret_cast<const udphdr*>(ptr);
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
        srcport() const{
            return ntohs(_chdr.uh_sport);
        }

        unsigned short
        dstport() const{
            return ntohs(_chdr.uh_dport);
        }

        unsigned short
        length() const{
            return ntohs(_chdr.uh_ulen);
        }

    };


// Specialize for TCP header

    template<>
    class header<hdr::tcp> {
    private:
        struct tcphdr _chdr;
        std::vector<u_char> _options; 

    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct tcphdr));
        }
        explicit header(const tcphdr* ptr) 
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Packet fragment too short");
            }
            _chdr = *ptr;
            if (_chdr.th_off > 5) {
                auto opt_begin = reinterpret_cast<const u_char*>(ptr) + 20;
                auto opt_len   = (_chdr.th_off << 2) - 20;
                auto opt_end = opt_begin + opt_len;
                _options.reserve(opt_len);
                std::copy(opt_begin,opt_end,_options.begin()); // Or copy to ... &_options[0]
            }
        }

        header(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(tcphdr)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            _chdr = *reinterpret_cast<const tcphdr*>(ptr);
            if (_chdr.th_off > 5) {
                auto opt_begin = reinterpret_cast<const u_char*>(ptr) + 20;
                auto opt_len   = (_chdr.th_off << 2) - 20;
                auto opt_end = opt_begin + opt_len;
                _options.reserve(opt_len);
                std::copy(opt_begin,opt_end,_options.begin()); // Or copy to ... &_options[0]
            }
        }

        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 
        unsigned short
        srcport() const{
            return ntohs(_chdr.th_sport);
        }

        auto
        c_hdr() const
        {
            return _chdr;
        } 
        unsigned short
        dstport() const{
            return ntohs(_chdr.th_dport);
        }

        unsigned short 
        hlen() const
        {
            // Returns the IP header length in bytes
            return static_cast<unsigned short>(_chdr.th_off << 2);
        }

        auto 
        options() const
        {
            return _options;
        }

    };


    // Specialize for ICMP header
    
    template<>
    class header<hdr::icmp> {
    private:
        struct icmp _chdr; 
    
    public:
        header()
        {
            memset(&_chdr, 0, sizeof(struct icmp));
        }
        explicit header(const struct icmp* ptr) 
        {
            if  ( ptr == nullptr )  
            {
                throw std::system_error(errno, std::system_category(), "Packet fragment too short");
            }
            _chdr = *ptr;
        }
    
        header(const u_char* ptr, ssize_t size) 
        {
            if ( (ptr == nullptr) || (size < sizeof(icmp)) ) {
                throw ( std::system_error(errno,std::system_category(),"Packet fragment too short") );
            }
            _chdr = *reinterpret_cast<const struct icmp*>(ptr);
        }
    
        header  (header const& rhs) = default;
        header& operator= (header const &) = default;
        header  (header&& rhs) = default;
        header& operator=(header &&) = default;
        ~header() = default; 
        unsigned short 
        type() const
        {
            return _chdr.icmp_type;
        }
    
        auto
        c_hdr() const
        {
            return _chdr;
        } 
        unsigned short 
        code() const
        {
            return _chdr.icmp_code;
        }
    };


}

#endif