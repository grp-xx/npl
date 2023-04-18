#include <algorithm>
#include <cctype>
#include <iostream>
#include "socket.hpp"
#include <sockaddress.hpp>
#include <sys/socket.h>
#include "utility.hpp"

int main() {

    npl::socket<AF_INET, SOCK_STREAM> sock;

    npl::sockaddress<AF_INET> srvAddr(10010);

    sock.bind(srvAddr);

    sock.listen();

    for(;;) {

        auto [connected, cltAddr] = sock.accept();
        std::cout << "Connected to client: " << cltAddr.host() << ":" << cltAddr.port() << std::endl;

        auto pid = fork();

        if (pid == 0)   // the child process
        {
            npl::buffer buf_len = connected.readn(4);
            auto mlen = npl::parseMsgHdr(buf_len);
            npl::buffer text = connected.readn(mlen);
            // for (auto &c : text) c = toupper(c);
            std::transform(text.begin(),text.end(),text.begin(),::toupper);
            connected.write(buf_len);   // The number of chars does not change!
            connected.write(text);
            connected.close();
            std::cout << "Client " << cltAddr.host() << ":" << cltAddr.port() << " Disconnected" << std::endl;  
            exit(0);
        }

        connected.close();
    }

    sock.close();
    return 0;
}