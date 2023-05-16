#ifndef _PCAP_FRAME_HPP_
#define _PCAP_FRAME_HPP_
#include <pcap/pcap.h>
#include <sys/types.h>
#include "frame.hpp"
#include "headers.hpp"


namespace npl::pcap {

    class frame : public npl::frame 
    {
    protected:
        const struct pcap_pkthdr* _pcap = nullptr;
    
    public:
        frame()
        : npl::frame()
        {}

        frame(const pcap_pkthdr* hdr, const u_char* ptr)
        : npl::frame(ptr, hdr->caplen)
        , _pcap(hdr)
        {}

        frame  (frame const& rhs) = default;
        frame& operator= (frame const &) = default;
        frame  (frame&& rhs) = default;
        frame& operator=(frame &&) = default;
        ~frame() = default;

        // getter to headers of the frame

        template<hdr h>
        auto get() const 
        {
            if constexpr ( h == hdr::pcap  )  return header<h>( _pcap  );  
            if constexpr ( h == hdr::ether )  return header<h>( _ether );  
            if constexpr ( h == hdr::vlan  )  return header<h>( _vlan  );
            if constexpr ( h == hdr::arp   )  return header<h>( _arp   );
            if constexpr ( h == hdr::ipv4  )  return header<h>( _ipv4  );
            if constexpr ( h == hdr::icmp  )  return header<h>( _icmp  );
            if constexpr ( h == hdr::udp   )  return header<h>( _udp   );
            if constexpr ( h == hdr::tcp   )  return header<h>( _tcp   );
        }

        // getter to raw pointers of the frame

        template<hdr h>
        auto c_addr() const 
        {
            if constexpr ( h == hdr::pcap  )  return _pcap ;  
            if constexpr ( h == hdr::ether )  return _ether;  
            if constexpr ( h == hdr::vlan  )  return _vlan ;
            if constexpr ( h == hdr::arp   )  return _arp  ;
            if constexpr ( h == hdr::ipv4  )  return _ipv4 ;
            if constexpr ( h == hdr::icmp  )  return _icmp ;
            if constexpr ( h == hdr::udp   )  return _udp  ;
            if constexpr ( h == hdr::tcp   )  return _tcp  ;
        }
        // Boolean methods to check if the frame carries the protocol p
        // Methods that check it the packet carries specific protocols..

        template <hdr h>
        bool has() const
        {
            if constexpr (h == hdr::pcap  )   return ( _pcap != nullptr  );
            if constexpr (h == hdr::ether )   return ( _ether != nullptr );
            if constexpr (h == hdr::vlan  )   return ( _vlan != nullptr  );
            if constexpr (h == hdr::arp   )   return ( _arp != nullptr   );
            if constexpr (h == hdr::ipv4  )   return ( _ipv4 != nullptr  );
            if constexpr (h == hdr::icmp  )   return ( _icmp != nullptr  );
            if constexpr (h == hdr::udp   )   return ( _udp != nullptr   );
            if constexpr (h == hdr::tcp   )   return ( _tcp != nullptr   );
        }  

    
    };

}


#endif