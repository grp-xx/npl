#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include "json.hpp"
#include "socket.hpp"
#include "sockaddress.hpp"
#include "chatroom.hpp"

npl::socket<AF_INET, SOCK_DGRAM> sock;
std::mutex m;

using json = nlohmann::json;

void msg_send(const message& msg, const npl::sockaddress<AF_INET>& dst ) 
{
    json msg_j = msg;
    std::string msg_str = msg_j.dump();
    // Send welcome message to connecting user                
    sock.sendto(npl::buffer(msg_str.begin(),msg_str.end()), dst);
}

message msg_rcv()
{
    auto [buf, srv] = sock.recvfrom(360);
    auto msg_str = std::string(buf.begin(),buf.end());
    json msg_j = json::parse(msg_str);
    message msg = msg_j;
    return msg; 
}

void transmitter(const npl::sockaddress<AF_INET>& srv_addr, const std::string& user)
{

    std::cin.ignore();

    for (;;) 
    {
        m.lock();
        std::cout << "<You>: ";
        m.unlock();
        std::string line;
        std::getline(std::cin,line);
        std::stringstream ss(line);            // Convert input line into a string stream (ingnore white spaces)
        std::string first, second, third;
        ss >> first;                           // retrieve first word (ignore white spaces)

        if (first == "#who") 
        {
            message msg {msg_type::C, "#who", "server", user, ""};
            msg_send(msg, srv_addr);
            continue;
        }

        if ( (first == "#bye") || (first == "#leave") ) 
        {
            message msg {msg_type::C, "#bye", "server", user, ""};
            msg_send(msg,srv_addr);
            continue;
        }

        if (first == "#pm") 
        {
            ss >> second >> third;             // extract second and third words (third word only to remove initial blanks in the text)
            std::string remaining;
            std::getline(ss, remaining);       // taking the remaining string line out of stream ss after the first, second and third words have been extracted. Extract the third to remove initial blanks. 
            message msg {msg_type::C, "#pm", second, user, third + remaining};
            msg_send(msg, srv_addr);
            continue;
        }

        // Default behavior
        message msg {msg_type::D, "", "all", user, line};
        msg_send(msg, srv_addr);
    }

}

void receiver(const std::string& user)
{
    for(;;)
    {
        message msg = msg_rcv();
        if (msg.from == user) continue;

        m.lock();
        std::string prompt = (msg.code == "#pm" ? msg.from + " (PM)" : msg.from);
        std::cout << std::endl << "<" << prompt << ">" << ": " << msg.text << std::endl;
        m.unlock();
        if (msg.code == "#byeOK") {
            exit(0);
        }
        m.lock();
        std::cout << "<You>: " << std::flush;
        m.unlock(); 
    
    }
}


int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <server> <port>" << std::endl;
        exit(1);
    }

    npl::sockaddress<AF_INET> srv_addr(argv[1], std::atoi(argv[2])); 

    std::string user;

    for (;;)
    {
        // Select user name
        std::cout << "Select user name: ";
        std::cin >> user;

         // Prepare join message
        message mjoin = {
            .type = msg_type::C, 
            .code = "#join",
            .to   = "server",
            .from =  user,
            .text = "Knock knock...",
        };       

        msg_send(mjoin, srv_addr);

        // Receive response...
        message response = msg_rcv();
        // Print Welcome/Refuse message from the system        
        std::cout << response.text << std::endl;

        if (response.code == "OK") 
            break;

    }

    std::thread rx(receiver, user);
    std::thread tx(transmitter, srv_addr, user);

    rx.join();
    tx.join();

    return 0;
}
