#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>

#include "socket.hpp"
#include "sockaddress.hpp"


int main()
{
    npl::socket<AF_INET, SOCK_STREAM> sock;
    // npl::sockaddress<AF_INET> srv_addr("10.114.62.226",15000);
    npl::sockaddress<AF_INET> srv_addr(16000);
    sock.bind(srv_addr);
    sock.listen();

    for(;;)
    {
        auto [conn, clt] = sock.accept();

        auto pid = fork();

        if (pid == 0) // Children process
        {
            std::cout << "Remote client " << clt.host() << " : " << clt.port() << " connected" << std::endl;
            for(;;)
            {
                auto buf = conn.read(256);
                if (buf.empty()) 
                    break;
                
                std::transform(buf.begin(),buf.end(),buf.begin(),::toupper);
                conn.write(buf);
            }
            conn.close();
            std::cout << "Remote client " << clt.host() << " : " << clt.port() << " disconnected" << std::endl;
            exit(EXIT_SUCCESS);
        }
        // Parent process
        conn.close();
    }
    
    sock.close();
    return EXIT_SUCCESS;
}