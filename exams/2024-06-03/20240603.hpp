#ifndef _20240603_HPP_
#define _20240603_HPP_


#include <cstddef>
#include <cstdint>
#include <functional>

struct tcpflowid_t {
    uint32_t srcip;
    uint16_t srcprt;
    uint32_t dstip;
    uint16_t dstprt;
}; 

namespace std {
    template <> 
    struct hash<tcpflowid_t> {
        size_t operator()(const tcpflowid_t& f) const {
            //    
            // Implement the hash function here
            // 
        };
    };

    template<>
    struct equal_to<tcpflowid_t>  {
        bool operator()(const tcpflowid_t& lhs, const tcpflowid_t& rhs) const {
            //    
            // Implement the bool operator here
            // 
        }
    };
}

#endif