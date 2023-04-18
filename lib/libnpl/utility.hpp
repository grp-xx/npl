#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_
#include "socket.hpp"
#include <algorithm>
#include <cstdint>

namespace npl {

    union MsgHdr {
        uint32_t    len;
        uint8_t  raw[4];
    };


    inline void addMsgHdr(buffer& buf) 
    {
        MsgHdr hdr {.len = htonl (buf.size() )};
        // std::copy(hdr.raw, hdr.raw+sizeof(uint32_t), buf.begin() );
        buf.insert(buf.begin(),hdr.raw,hdr.raw+sizeof(uint32_t));
    }

    inline uint32_t parseMsgHdr(const buffer& buf)
    {
        MsgHdr hdr;
        std::copy(buf.begin(),buf.end(),hdr.raw);
        return ntohl( hdr.len );
    }

}




#endif