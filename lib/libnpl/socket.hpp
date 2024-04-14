#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_
#include <cstdint>
#include <system_error>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include "sockaddress.hpp"

namespace npl {

typedef std::vector<uint8_t> buffer;

template<int F, int type>
class socket {
private:
    int _sockfd;

public:
    explicit socket(int protocol = 0)
    {
        if ( ( _sockfd = ::socket(F,type,protocol)) == -1) {
            throw std::system_error(errno, std::generic_category(), "socket");
        }
    }

    socket(const socket& ) = delete;
    socket& operator=(const socket&) = delete;

    socket(socket&& rhs)
    : _sockfd(rhs._sockfd)
    {
        rhs._sockfd = -1;
    }

    socket& operator=(socket&& rhs)
    {
        if (*this != rhs)
        {
            this->close();
            _sockfd = rhs._sockfd;
            rhs._sockfd = -1;
        }
        return *this;
    }
    
    ~socket()
    {
        if (_sockfd != -1) {
            ::close(_sockfd);
        }
    }


    void close()
    {
        if (_sockfd != -1)
        {
            ::close(_sockfd);
            _sockfd = -1;
        }
    }


    void bind(const sockaddress<F>& addr)
    {

    }







};





}


#endif