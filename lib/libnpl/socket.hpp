#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <unistd.h>
#include <utility>
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
        if ((::bind(_sockfd, &addr.c_addr(), addr.len()) ) ==-1) {
            throw std::system_error(errno,std::system_category(),"bind");
        }
    }

    void listen(int backlog = 5)
    {
        if (::listen(_sockfd, backlog ) ==-1) {
            throw std::system_error(errno,std::system_category(),"listen");
        }
    }
        
    std::pair< socket, sockaddress<F> > accept()
    {
        sockaddress<F> peer;
        socket accepted;
        if ( (accepted._sockfd = ::accept(_sockfd, &peer.c_addr(), &peer.len())) == -1 ) {
            throw std::system_error(errno,std::system_category(),"accept");
        }
        return std::make_pair(std::move(accepted), peer);
    }
        
    void connect(const sockaddress<F>& remote)
    {
        if ((::connect(_sockfd, &remote.c_addr(), remote.len()) ) ==-1) {
            throw std::system_error(errno,std::system_category(),"connect");
        }
    }

    // I/O methods

    std::ptrdiff_t write(const buffer& buf) const
    {
        return ::write(_sockfd, &buf[0], buf.size());
    }

    std::ptrdiff_t read(buffer& buf) const
    {
        return ::read(_sockfd, &buf[0], buf.size());
    }

    buffer read(size_t n) const 
    {
        buffer buf(n);
        int nbytes = ::read(_sockfd, &buf[0], buf.size());
        return buffer(buf.begin(),buf.begin()+nbytes);
    }

    std::ptrdiff_t 
    sendto(const buffer& buf, const sockaddress<F>& remote, int flags = 0) const 
    {
        return ::sendto(_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), remote.len());
    }

    std::ptrdiff_t
    recvfrom(buffer& buf, sockaddress<F>& remote, int flags = 0) const
    {
        return ::recvfrom(_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), &remote.len());
    }

    std::pair<buffer, sockaddress<F>>
    recvfrom(size_t n, int flags = 0) const
    {
        buffer buf(n);
        sockaddress<F> remote;
        int nbytes = ::recvfrom(_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), &remote.len());
        return std::make_pair(buffer(buf.begin(),buf.begin()+nbytes),remote);
    }





};





}


#endif