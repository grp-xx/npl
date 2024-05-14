#ifndef _PCAP_HPP_
#define _PCAP_HPP_

#include <pcap/pcap.h>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <system_error>
#include <utility>


enum capture {live, offline};

namespace npl::pcap {

    template<typename F>
    void callback(u_char* user, const struct pcap_pkthdr* hdr, const u_char* bytes)
    {
        auto that = reinterpret_cast<F*>(user);
        that->operator()(hdr, bytes);
    }

    template <capture mode>
    class reader
    {
    private:
        pcap_t*    _handle = nullptr;
        std::string  _dev;
        bpf_program _prog = {0,nullptr};

//    public:
//        reader(std::string device = "any", int snaplen = 64,  int promisc = 1, int to_ms = 100)  requires (mode == live)  // Thanks to C++20 concepts...
//        {
//            char errbuf[PCAP_ERRBUF_SIZE];
//            if ( (_handle = pcap_open_live(device.c_str(), snaplen, promisc, to_ms, errbuf )) == nullptr) 
//            {
//                throw std::system_error(errno, std::system_category(), "Failed to open device: " + std::string(errbuf) );
//            }
//        }
    public:

// New API

//        reader() requires (mode == live)   // C++ 20 concept feature
//        {
//            char errbuf[PCAP_ERRBUF_SIZE];
//            if ( ( _handle = pcap_create("any", errbuf) ) == nullptr )
//            {
//                throw std::system_error(errno,std::system_category(),"Failed to open network device: " + std::string(errbuf));
//            }
//        }

        explicit reader(std::string device = "any") requires (mode == live)
        {
            char errbuf[PCAP_ERRBUF_SIZE];
            if ( ( _handle = pcap_create(device.c_str(), errbuf) ) == nullptr )
            {
                throw std::system_error(errno,std::system_category(),"Failed to open network device: " + std::string(errbuf));
            }
            _dev = device;
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
        ,_dev(other._dev)
        ,_prog(other._prog)
        {
            other._handle = nullptr;
            other._dev.clear();
            other._prog.bf_len = 0;
            other._prog.bf_insns = nullptr;
        }

        reader& operator=(reader&& other)
        {
            if ( *this != other)
            {
                this->close();
                _handle = other._handle;
                _dev = other._dev;
                pcap_freecode(&_prog);
                _prog = other._prog;
                other._handle = nullptr;
                other._dev.clear();
                other._prog.bf_len = 0;
                other._prog.bf_insns = nullptr;
                return *this;
            }
        }

        ~reader()
        {
            this->close();
            if (_prog.bf_insns != nullptr) 
            {
               pcap_freecode(&_prog);
            }
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

        // New API
        int
        activate()  const
        {
            int out = pcap_activate(_handle);
            if ( out < 0  )
            {
                throw std::system_error(errno,std::system_category(),"Failed to activate capture device: " + std::to_string(out));
            }
            if ( out  > 0  )
            {
                throw std::system_error(errno,std::system_category(),"Warning: activate capture device: " + std::to_string(out));
            }
            return out;
        }

        int
        promisc()  const
        {
            int out = pcap_set_promisc(_handle, 1);
            if ( out != 0  )
            {
                throw std::system_error(errno,std::system_category(),"Failed to set device in promiscous mode ");
            }
            return out;
        }


        int
        snaplen(int caplen) const
        {
            int out = pcap_set_snaplen(_handle, caplen);
            if ( out != 0  )
            {
                throw std::system_error(errno,std::system_category(),"Failed to set snaplen ");
            }
            return out;
        }


        int
        set_timeout(int to_ms) const
        {
            int out = pcap_set_timeout(_handle, to_ms);
            if ( out != 0  )
            {
                throw std::system_error(errno,std::system_category(),"Failed to set timeout ");
            }
            return out;
        }
        
        int
        buffer_size(int n) const
        {
            int out = pcap_set_timeout(_handle, n);
            if ( out != 0  )
            {
                throw std::system_error(errno,std::system_category(),"Failed to set buffer size ");
            }
            return out;
        }
        
        int
        immediate_mode() const
        {
            int out = pcap_set_immediate_mode(_handle, 1);
            if ( out != 0  )
            {
                throw std::system_error(errno,std::system_category(),"Failed to set immediate mode ");
            }
            return out;
        }

        void
        rfmon()  const
        {
            if ( pcap_can_set_rfmon(_handle) )  
            {
                if (pcap_set_rfmon(_handle, 1) != 0)
                {
                    throw std::system_error(errno,std::system_category(),"Failed to enable rfmon mode ");
                }
            }
        }

        // Activate device with default values
        void 
        open() const
        {
            this->snaplen(64);
            this->set_timeout(100);
            this->promisc();
            // tap.buffer_size(1);
            // tap.immediate_mode();
            this->rfmon();
            this->activate();
        }



        // I/O methods

        std::string
        device() const
        {
            return _dev;
        }

        const bpf_program & 
        get_bpf_program() const
        {
            return _prog;
        }

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

        template<typename F>
        int loop(F& func, int cnt = -1) const
        {
            int out;
            if ( (out = pcap_loop(_handle, cnt, callback<F>, reinterpret_cast<u_char*>(&func))) == -1 )
            {
                throw std::runtime_error("pcap_loop error");
            }
            return out;
        }


        template<typename F>
        int dispatch(F& func, int cnt = -1) const
        {
            int out;
            if ( (out = pcap_dispatch(_handle, cnt, callback<F>, reinterpret_cast<u_char*>(&func))) == -1 )
            {
                throw std::runtime_error("pcap_dispatch error");
            }
            return out;
        }

        // Add BPF stuff...
        
        void
        filter(std::string rule)
        {
            bpf_u_int32 net;
            bpf_u_int32 mask = PCAP_NETMASK_UNKNOWN;
            char errbuf[PCAP_ERRBUF_SIZE];

            if ( (_dev) != "any" && !_dev.empty() )
            {
                if ( pcap_lookupnet(_dev.c_str(), &net, &mask, errbuf) != 0) 
                {
                    mask = PCAP_NETMASK_UNKNOWN;
                };
            }

            if (pcap_compile(_handle, &_prog, rule.c_str(), 1, mask) != 0)
            {
                throw std::runtime_error("BPF filter can't be compiled. " + std::string(pcap_geterr(_handle)));
            }

            if (pcap_setfilter(_handle, &_prog) != 0 )
            {
                throw std::runtime_error("BPF filter can't be set. " + std::string(pcap_geterr(_handle)));
            }

        }

        void 
        print_bpf_program() const
        {
            bpf_dump(&_prog, 1);
        }

    };

    







}


#endif