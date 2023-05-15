#ifndef _PCAP_HPP_
#define _PCAP_HPP_

#include <pcap/pcap.h>
#include <sys/types.h>
#include <system_error>
#include <utility>


enum capture {live, offline};

namespace npl::pcap {

    template <capture mode>
    class reader
    {
    private:
        pcap_t* _handle = nullptr;

    public:
        reader(std::string device = "any", int snaplen = 64,  int promisc = 1, int to_ms = 100)  requires (mode == live)  // Thanks to C++20 concepts...
        {
            char errbuf[PCAP_ERRBUF_SIZE];
            if ( (_handle = pcap_open_live(device.c_str(), snaplen, promisc, to_ms, errbuf )) == nullptr) 
            {
                throw std::system_error(errno, std::system_category(), "Failed to open device: " + std::string(errbuf) );
            }
        }

        reader(std::string filename)  requires (mode == offline)  // Thanks to C++20 concepts...
        {
            char errbuf[PCAP_ERRBUF_SIZE];
            if ( (_handle = pcap_open_offline(filename.c_str(), errbuf )) == nullptr) 
            {
                throw std::system_error(errno, std::system_category(), "Failed to open file: " + std::string(errbuf) );
            }
        }

        reader(const reader& ) = delete;
        reader& operator=(const reader&) = delete;

        reader(reader&& other)
        :_handle(other._handle)
        {
            other._handle = nullptr;
        }

        reader& operator=(reader&& other)
        {
            if ( *this != other)
            {
                this->close();
                _handle = other._handle;
                other._handle = nullptr;
                return *this;
            }
        }

        ~reader()
        {
            this->close();
        }

        void close()
        {
            if (_handle != nullptr)
            {
                pcap_close(_handle);
            }
        }

        operator bool()
        {
            return (_handle != nullptr);
        }

        // I/O methods

        int datalink() const
        {
            if (_handle != nullptr) 
            {
                return pcap_datalink(_handle);
            }
        }

        std::pair<struct pcap_pkthdr, const u_char*> 
        next() const
        {
            struct pcap_pkthdr hdr;
            const u_char* ptr = pcap_next(_handle, &hdr);
            return std::make_pair(hdr, ptr);  
        }










    };

    







}


#endif