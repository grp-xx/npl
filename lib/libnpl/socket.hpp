#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <utility>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include "sockaddress.hpp"

namespace npl {

typedef std::vector<uint8_t> buffer;

template<int F, int type>
class socket 
{
private:
    int _sockfd;

public:
    explicit socket(int protocol = 0) 
    {
        if ( (_sockfd = ::socket(F, type, protocol) ) == -1 ) {
            throw std::system_error(errno, std::generic_category(),"socket");
        }
    }

    socket(const socket&) = delete;
    socket& operator=(const socket&) = delete;

    socket(socket&& rhs) 
    :_sockfd(rhs._sockfd)
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
        if (_sockfd != - 1) 
        {
            this->close();
        }
    }

    // Connection management 

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
        if constexpr (F == AF_UNIX) {
            unlink(addr.name().c_str());
        }
        if (::bind(_sockfd, &addr.c_addr(), addr.len()) == -1 ) {
            throw std::system_error(errno, std::system_category(),"bind");
        }
    }
        
    void listen(int backlog = 5) 
    {
        if (::listen(_sockfd, backlog) == -1 ) {
            throw std::system_error(errno, std::system_category(),"listen");
        }
    }
        
    void connect(const sockaddress<F>& remote)
    {
        if (::connect(_sockfd, &remote.c_addr(), remote.len()) == -1 ) {
            throw std::system_error(errno, std::system_category(),"connect");
        }
    }
    
    
    
    std::pair< socket, sockaddress<F> > accept()
    {
        sockaddress<F> peer;
        socket accptd;
        if ( ( accptd._sockfd = ::accept(_sockfd, &peer.c_addr(), &peer.len()) ) == -1 ) 
        {
            throw std::system_error(errno, std::system_category(),"accept");
        }
        return std::make_pair(std::move(accptd), peer);
    }


    // I/O methods

    std::ptrdiff_t write(const buffer& buf) const
    {
        return ::write(_sockfd, &buf[0], buf.size());
    }

    std::ptrdiff_t     writen(const buffer& buf, ssize_t n)
    {
        std::ptrdiff_t num_written;
        std::ptrdiff_t tot_written = 0;

        while (tot_written < n ) {
            num_written = ::write(_sockfd, &buf[tot_written], n-tot_written); 

            if (num_written <= 0)  // An error has happened
            {
                if (num_written == -1 && errno == EINTR) 
                    continue;     // Restart writing bythes form scratch in case of interrupt
                else 
                    return -1;  
            }  
            tot_written += num_written;
        }
        return tot_written;
    }

    std::ptrdiff_t      send(const buffer& buf, int flags = 0) const
    {
       return ::send(_sockfd, &buf[0], buf.size(), flags);
    }


    std::ptrdiff_t sendto(const buffer& buf, const sockaddress<F>& remote, int flags = 0) const
    {
        return ::sendto(_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), remote.len());
    }



    std::ptrdiff_t read(buffer& buf) const
    {
        return ::read(_sockfd, &buf[0], buf.size());
    }

    std::ptrdiff_t     readn(buffer& buf, ssize_t n ) const
    {
        std::ptrdiff_t num_read;
        std::ptrdiff_t tot_read = 0;

        while(tot_read < n)
        {
            num_read = ::read(_sockfd, &buf[tot_read], n - tot_read);

            if (num_read == 0)
                return tot_read;  // EOF reached;

            if (num_read == - 1)  // An error has happened
            {
                if (errno == EINTR) 
                    continue;     // Restart reading bytes in case of interrupt
                else 
                    return -1;  
            } 
            tot_read+= num_read;

        }
        return tot_read;
    }

    buffer read(int n) const 
    {
        buffer buf(n);
        std::ptrdiff_t nbytes = this->read(buf);
        return buffer(buf.begin(),buf.begin()+nbytes);
    }

    buffer     readn(ssize_t len ) const
    {
        buffer buf(len);
        auto tot_read = this->readn(buf,len);
        return buffer(buf.begin(),buf.begin()+tot_read);
    }
        std::ptrdiff_t      recv(buffer& buf, int flags = 0) const
        {
            return ::recv(_sockfd, &buf[0], buf.size(), flags);
        }

    buffer              recv(int len, int flags = 0) const
    {
        buffer buf(len);
        std::ptrdiff_t n = ::recv(_sockfd, &buf[0], buf.size(), flags);
        return buffer(buf.begin(),buf.begin()+n);
    }

    std::ptrdiff_t      recvn(buffer& buf, int flags = 0) const
    {
        return ::recv(_sockfd, &buf[0], buf.size(), flags | MSG_WAITALL);
    }

    
    buffer              recvn(int len, int flags = 0) const
    {
        buffer buf(len);
        std::ptrdiff_t n = ::recv(_sockfd, &buf[0], buf.size(), flags | MSG_WAITALL);
        return buffer(buf.begin(),buf.begin()+n);
    }

    std::ptrdiff_t  recvfrom(buffer& buf, int flags, sockaddress<F>& remote) const
    {
        return ::recvfrom(_sockfd, &buf[0], buf.size(), flags, &remote.c_addr(), &remote.len());
    }

    std::pair< buffer, sockaddress<F> > recvfrom(int len, int flags = 0) const
    {
        buffer buf(len);
        sockaddress<F> remote;
        std::ptrdiff_t nbytes = this->recvfrom(buf,flags,remote);
        return std::make_pair(buffer(buf.begin(),buf.begin()+nbytes), remote);
    }
    
    
    // Advanced socket methods (To be reviewed and made better possibly)

    sockaddress<F> 
    getsockname() const
    {
        sockaddress<F> name;
        if (::getsockname(_sockfd, &name.c_addr(), &name.len() ) !=0 ) {
           throw std::system_error(errno,std::generic_category(),"getsockname");
        }
        return name;
    }

    sockaddress<F>
    getpeername() const
    {
        sockaddress<F> name;
        if (::getpeername(_sockfd, &name.c_addr(), &name.len() ) !=0 ) {
           throw std::system_error(errno,std::generic_category(),"getpeername");
        }
        return name;
    }

    // Setting and retrieving socket options
    int
    setsockopt(int level, int optname, const void *optval, socklen_t optlen)
    {
        int out = ::setsockopt(_sockfd, level, optname, optval, optlen);
        if (out == -1) {
            throw std::system_error(errno,std::generic_category(),"setsockopt");
        }
        return out;
    }

    int
    getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
    {
        int out = ::getsockopt(_sockfd, level, optname, optval, optlen);
        if (out == -1) {
            throw std::system_error(errno,std::generic_category(),"getsockopt");
        }
        return out;
    }

    int set_reuseaddr() 
    {
       int optval = 1;
       int out = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
       if (out == -1) {
          throw std::system_error(errno,std::generic_category(),"set_reuseaddr");
       }
       return out;
    }

    int broadcast_enable() 
    {
       int optval = 1;
       int out = ::setsockopt(_sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
       if (out == -1) {
          throw std::system_error(errno,std::generic_category(),"broadcast_enable");
       }
       return out;
    }       

         #ifdef __linux__
        // Put interface in promisc mode
        
        int set_promisc(int ifindex) 
        {
            struct packet_mreq mreq = {0};
            mreq.mr_ifindex = ifindex;
            mreq.mr_type = PACKET_MR_PROMISC;
            int out = ::setsockopt(_sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
            if (out == -1) {
                throw std::system_error(errno,std::generic_category(),"set_promisc");
            }
            return out;
        }

        int set_promisc(std::string device)
        {
            int if_index = if_nametoindex(device.c_str());
            if ( if_index == 0 )
            {
                throw std::system_error(errno, std::system_category(), "Interface not available");
            }  
            return this->set_promisc(if_index);
        }


        // Enable fanout mode (join fanout group)
        int set_fanout(int group, int mode = PACKET_FANOUT_HASH)
        {
            int arg = ( (mode << 16) | group);
            int out = ::setsockopt(_sockfd, SOL_PACKET, PACKET_FANOUT, &arg, sizeof(arg));
            if (out == -1) {
                throw std::system_error(errno,std::generic_category(),"set_fanout");
            }
            return out;
        }

    #endif 




};



}





#endif