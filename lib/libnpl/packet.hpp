#ifndef _PACKET_HPP_
#define _PACKET_HPP_

#include "headers.hpp"
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace npl {

    template <hdr h>
    class packet {
    private:
        const u_int8_t* _base;
        u_int16_t _length;
        std::vector<std::pair<hdr,u_int16_t>> _protocols;

    public:
        packet(const u_int8_t* ptr, u_int16_t caplen)
        : _base(ptr), _length(caplen)
        {
                auto next_hdr = h;
                u_int16_t offset = 0;

                while (next_hdr != hdr::unkown)
                {
                    switch (next_hdr) {

                        case hdr::ether: // Ethernet
                        {
                            auto current_ptr = _base + offset;
                            auto hdr_size = sizeof(ether_header);
                            auto current_proto = hdr::ether;

                            if ((current_ptr == nullptr) || (caplen < hdr_size))
                                return;
                            
                            auto hdr_ptr = reinterpret_cast<const ether_header*>(current_ptr);


                            if (hdr_ptr->ether_type == ntohs(ETHERTYPE_VLAN)) {
                                next_hdr = hdr::vlan;
                                break;
                            } 

                            _protocols.push_back(std::make_pair(current_proto, offset));
                            offset += hdr_size;
                            caplen = _length - offset;

                            if (hdr_ptr->ether_type == ntohs(ETHERTYPE_IP)) {
                                next_hdr = hdr::ipv4;
                                break;
                            } 

                            if (hdr_ptr->ether_type == ntohs(ETHERTYPE_ARP)) {
                                next_hdr = hdr::arp;
                                break;
                            }
                            next_hdr = hdr::unkown; 
                            break;
                        }

                        case hdr::vlan: 
                        {
                            auto current_ptr = _base + offset;
                            auto hdr_size = sizeof(vlan_header);
                            auto current_proto = hdr::vlan;

                            if ((current_ptr == nullptr) || (caplen < hdr_size))
                                return;
                            
                            auto hdr_ptr = reinterpret_cast<const vlan_header*>(current_ptr);

                            _protocols.push_back(std::make_pair(current_proto, offset));
                            offset += hdr_size;
                            caplen = _length - offset;

                            if (hdr_ptr->ether_type == ntohs(ETHERTYPE_IP)) {
                                next_hdr = hdr::ipv4;
                                break;
                            } 

                            if (hdr_ptr->ether_type == ntohs(ETHERTYPE_ARP)) {
                                next_hdr = hdr::arp;
                                break;
                            } 
                            next_hdr = hdr::unkown; 
                            break;
                        }        

    // ARP
                        case hdr::arp: 
                        {
                            auto current_ptr = _base + offset;
                            auto hdr_size = sizeof(arphdr);
                            auto current_proto = hdr::arp;

                            if ((current_ptr == nullptr) || (caplen < hdr_size))
                                return;
                            
                            auto hdr_ptr = reinterpret_cast<const arphdr*>(current_ptr);

                            _protocols.push_back(std::make_pair(current_proto, offset));
                            offset += hdr_size;
                            caplen = _length - offset;

                            next_hdr = hdr::unkown;
                            break;
                        }      

                        case hdr::ipv4: 
                        {
                            auto current_ptr = _base + offset;
                            auto hdr_size = sizeof(ip);
                            auto current_proto = hdr::ipv4;

                            if ((current_ptr == nullptr) || (caplen < hdr_size))
                                return;
                            
                            auto hdr_ptr = reinterpret_cast<const ip*>(current_ptr);

                            _protocols.push_back(std::make_pair(current_proto, offset));
                            auto iphl = (hdr_ptr->ip_hl << 2);
                            offset += iphl;
                            caplen = _length - offset;

                            switch (hdr_ptr->ip_p) {
                                case IPPROTO_ICMP:
                                {
                                    next_hdr = hdr::icmp;
                                    break;
                                }


                                case IPPROTO_UDP:
                                {
                                    next_hdr = hdr::udp;
                                    break;
                                }

                                case IPPROTO_TCP:
                                {
                                    next_hdr = hdr::tcp;
                                    break;
                                }
                                default:
                                {
                                    next_hdr = hdr::unkown;
                                    break;
                                }
                            
                            }
                            break;
                        }      

                        case hdr::udp: {
                            auto current_ptr = _base + offset;
                            auto hdr_size = sizeof(udphdr);
                            auto current_proto = hdr::udp;

                            if (current_ptr == nullptr || caplen < hdr_size) return;

                            _protocols.push_back(std::make_pair(current_proto, offset));
                            offset += hdr_size; 
                            caplen = _length - offset;

                            auto hdr_ptr = reinterpret_cast<const udphdr*>(current_ptr);
                            next_hdr = hdr::unkown;
                            break;
                        }

                        case hdr::tcp: {
                            auto current_ptr = _base + offset;
                            auto hdr_size = sizeof(tcphdr);
                            auto current_proto = hdr::tcp;

                            if (current_ptr == nullptr || caplen < hdr_size) return;

                            auto hdr_ptr = reinterpret_cast<const tcphdr*>(current_ptr);
                            _protocols.push_back(std::make_pair(current_proto, offset));
                            auto tcphl = (hdr_ptr->th_off << 2);
                            offset += tcphl; //+ options...
                            caplen = _length - offset;

                            next_hdr = hdr::unkown;
                            break;
                        }

                        case hdr::icmp: {
                            auto current_ptr = _base + offset;
                            auto hdr_size = sizeof(icmp);
                            auto current_proto = hdr::icmp;

                            if (current_ptr == nullptr || caplen < hdr_size) return;

                            _protocols.push_back(std::make_pair(current_proto, offset));
                            offset += hdr_size;
                            caplen = _length - offset;
                            
                            next_hdr = hdr::unkown;
                            break;
                        }
                        
                        default: {
                            next_hdr = hdr::unkown;
                            break;
                        }  
                    }
                }
            }

        packet(const packet&) = default;
        packet& operator=(const packet&) = default;
        packet(packet&& ) = default;
        packet& operator=(packet&) = default;
        ~packet() = default;

        
        // Bool methods that check if packet carries a specific protocol with header h
        template<hdr proto>
        auto has() const
        {
            int out = 0;
            for (auto &x : _protocols)
            {
                if (x.first == proto) 
                    ++out;
            }
            return out;
        }

        // returns proto headers
        template<hdr proto>
        auto get() const 
        {
            std::vector<header<proto>> out;
            for (auto &x : _protocols)
            {
                if (x.first == proto) 
                    out.push_back(header<proto>(_base+x.second,_length-x.second));
            }
            return out;
        }

        // Returns the whole sequence of headers
        auto dump() const 
        {
            return _protocols;
        }

        void display() const
        {
            // for (auto &x : _protocols) std::cout << PROTOCOL_NAME.at(x.first) << "  ";
            // std::cout << std::endl;

            auto print = [](auto p) {
                std::cout << PROTOCOL_NAME.at(p.first) << "--";
            };

            std::for_each(_protocols.begin(), _protocols.end()-1, print);
            std::cout << PROTOCOL_NAME.at(_protocols.back().first);
        }
    };


}


#endif