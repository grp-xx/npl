#ifndef _FRAME_HPP_
#define _FRAME_HPP_

#include <cstdio>
#include <net/ethernet.h>          // -> Ethernet, MAC 802.3
#include <net/if_arp.h>            // -> ARP
#include <netinet/ip.h>         	  // -> IPv4
#include <netinet/udp.h>           // -> UDP
#include <netinet/tcp.h>           // -> TCP
#include <netinet/ip_icmp.h>       // -> ICMP
#include <sys/types.h>
#include "headers.hpp"

namespace npl {

    class frame 
    {
    protected:
        const ether_header*    _ether = nullptr;
        const vlan_header*      _vlan = nullptr;
        const arphdr*            _arp = nullptr;
        const ip*               _ipv4 = nullptr;
        const udphdr*            _udp = nullptr;
        const tcphdr*            _tcp = nullptr; 
        const icmp*             _icmp = nullptr;
    
    
    public:
        frame(){};

        frame(const u_char* ptr, ssize_t caplen)
        {
            if (ptr == nullptr || caplen < sizeof(ether_header)) return;
            // Assume initiale the frame is plain Ethernet
            _ether = reinterpret_cast<const ether_header*>(ptr);

            if ( _ether->ether_type == htons(ETHERTYPE_VLAN) ) 
            {
                // VLAN detected
                if ( caplen < sizeof(vlan_header) ) return;
                _vlan = reinterpret_cast<const vlan_header*>(_ether);
                _ether = nullptr;
                caplen -= sizeof(vlan_header);
            }
            else 
            {
                // Frame is plain Ethernet
                caplen -= sizeof(ether_header);
            }

            // Now, what's next? Which protocol is carried by L2?

            if (_ether) 
            {
                if (_ether->ether_type == htons(ETHERTYPE_ARP))
                {
                    if (caplen < sizeof(struct arphdr)) return;
                    _arp = reinterpret_cast<const struct arphdr*>(_ether+1);
                    // caplen -= sizeof(struct arphdr);
                }

                if (_ether->ether_type == htons(ETHERTYPE_IP))
                {
                    if (caplen < sizeof(struct ip)) return;
                    _ipv4 = reinterpret_cast<const struct ip*>(_ether+1);
                    // caplen -= (_ipv4->ip_hl << 2);
                }
            } 
            else
            // L2 is VLAN 
            {
                if (_vlan->ether_type == htons(ETHERTYPE_ARP))
                {
                    if (caplen < sizeof(struct arphdr)) return;
                    _arp = reinterpret_cast<const struct arphdr*>(_vlan+1);
                    // caplen -= sizeof(struct arphdr);
                }

                if (_vlan->ether_type == htons(ETHERTYPE_IP))
                {
                    if (caplen < sizeof(struct ip)) return;
                    _ipv4 = reinterpret_cast<const struct ip*>(_vlan+1);
                    // caplen -= (_ipv4->ip_hl << 2);
                }
            } 

            if (_ipv4)
            {
                ssize_t iphl = ( _ipv4->ip_hl << 2 );
                auto ip4_ptr= reinterpret_cast<const u_char*>(_ipv4);
                if (caplen < iphl) return;
                caplen -= iphl;

                switch (_ipv4->ip_p) 
                {
                    case IPPROTO_ICMP: 
                                        {
                                            if ( caplen < sizeof(struct icmp) ) return;
                                            _icmp = reinterpret_cast<const struct icmp*>(ip4_ptr+iphl); 
                                        }

                    case IPPROTO_UDP:
                                        {
                                            if ( caplen < sizeof(struct udphdr) ) return;
                                            _udp = reinterpret_cast<const struct udphdr*>(ip4_ptr+iphl); 
                                        }
                    case IPPROTO_TCP:
                                        {
                                            if ( caplen < sizeof(struct tcphdr) ) return;
                                            _tcp = reinterpret_cast<const struct tcphdr*>(ip4_ptr+iphl); 
                                        }
                }
            }

        } // End frame ctor            


        frame(const frame&) = default;
        frame& operator=(const frame&) = default;
        frame(frame&& ) = default;
        frame& operator=(frame&) = default;
        ~frame() = default;

        // Bool methods that check if frame carries a specific protocol
        template<hdr h>
        auto has() const
        {
            if constexpr (h == hdr::ether) return  (_ether != nullptr  );
            if constexpr (h == hdr::vlan)  return  ( _vlan != nullptr  );
            if constexpr (h == hdr::arp)   return  ( _arp  != nullptr  );
            if constexpr (h == hdr::ipv4)  return  ( _ipv4 != nullptr  );
            if constexpr (h == hdr::icmp)  return  ( _icmp != nullptr  );
            if constexpr (h == hdr::udp)   return  ( _udp  != nullptr  );
            if constexpr (h == hdr::tcp)   return  ( _tcp  != nullptr  );
        }
        
        // Getter methods
        template<hdr h>
        auto get() const
        { 
            if constexpr (h == hdr::ether) return     header<hdr::ether>(_ether);
            if constexpr (h == hdr::vlan)  return    header<hdr::vlan>(_vlan);
            if constexpr (h == hdr::arp)   return      header<hdr::arp>(_arp);
            if constexpr (h == hdr::ipv4)  return    header<hdr::ipv4>(_ipv4);
            if constexpr (h == hdr::icmp)  return    header<hdr::icmp>(_icmp);
            if constexpr (h == hdr::udp)   return      header<hdr::udp>(_udp);
            if constexpr (h == hdr::tcp)   return      header<hdr::tcp>(_tcp);
        }

        // Getter methods returning the pointers to the C headers
        template<hdr h>
        auto c_addr() const
        { 
            if constexpr (h == hdr::ether) return     _ether;
            if constexpr (h == hdr::vlan)  return      _vlan;
            if constexpr (h == hdr::arp)   return       _arp;
            if constexpr (h == hdr::ipv4)  return      _ipv4;
            if constexpr (h == hdr::icmp)  return      _icmp;
            if constexpr (h == hdr::udp)   return       _udp;
            if constexpr (h == hdr::tcp)   return       _tcp;
        }
        };


}







#endif