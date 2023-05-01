#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <iomanip>
#include <utility>
#include "json.hpp"
#include "socket.hpp"
#include "sockaddress.hpp"
#include "chatroom.hpp"

using json = nlohmann::json;


int main() 
{
    // std::string srv_ip = "192.168.1.34";
    std::string srv_ip = "";
    int srv_port       = 10000;
    auto srv_addr      = npl::sockaddress<AF_INET>(srv_ip,srv_port); 

    npl::socket<AF_INET, SOCK_DGRAM> sock;
    sock.bind(srv_addr);

    std::map< std::string, npl::sockaddress<AF_INET> > cmap;

    //Lambda for sending message
    auto msg_send = [&sock](const message& msg, const npl::sockaddress<AF_INET>& dst ) 
                        {
                        json msg_j = msg;
                        std::string msg_str = msg_j.dump();

                        // Send welcome message to connecting user                
                        sock.sendto(npl::buffer(msg_str.begin(),msg_str.end()), dst);
                        };

    for (;;) 
    {
        // Receive message
        auto [buf, client] = sock.recvfrom(360);
        json msg_j = json::parse( std::string(buf.begin(),buf.end()) );

        // construct message object from json representation 
        message msg = msg_j;

        

        if (msg.code == "#join") 
        {
            // send response to join message
            message response = {
                .type = msg_type::C,
                .to   = msg.from,
            };

            if (cmap.find(msg.from) == cmap.end())  // User with new identifier 
            {
                cmap[msg.from] = client;
                response.code  = "OK";
                response.text  = "Welcome to the NPL chat room!!";

                msg_send(response, client);
                // Send welcome message to connecting user                



                // Prepare message
                message to_all {
                    .type = msg_type::D,
                    .code = "",
                    .to   = "all",
                    .from = "Server",
                    .text = msg.from + " has joined the chatroom",
                };

                std::cout << to_all.text << std::endl;  // Output to stdout the new joining user
                        
                // Notify all but the new arrived 
                for (auto [k,dest] : cmap) 
                {
                    if (k != msg.from)
                    {
                        msg_send(to_all, dest);
                    }
                }                                       

            }
            else // There was already a user with the same user name 
            {
                // Prepare message of reject
                response.code = "NO";
                response.text = "Access refused, existing username";
                // Send welcome message to connecting user 
                msg_send(response, client); 
            }
            continue;            
        }

        if ( (msg.code == "#bye") || (msg.code == "#leave") ) 
        {
            cmap.erase(msg.from);  //Remove client key from the map
            std::cout << msg.from << " has left the chat room" <<std::endl; // Prin to to screen locally on server
            // Send bye to the leaving user
            message mbye = {msg_type::D, "#byeOK", msg.from, "Server", "Bye bye " + msg.from};
            msg_send(mbye, client);
            // Notify all that the user has left the room
            message mbyeall = {msg_type::D, "", "all", "Server", msg.from + " has left the chat room"};
            for (auto [k,dest] : cmap) 
            {
                msg_send(mbyeall, dest);
            }
            continue;                                       
        }

        if (msg.code == "#who") 
        {
            std::stringstream ss;
            for (auto i : cmap) 
                ss << i.first << ", ";
            
            std::string list = ss.str();
            message response = {msg_type::D, "OK", msg.from, "Server", "Users in the room: " + std::string(list.begin(),list.end()-2)};  // remove last two charcters ", "
            msg_send(response, client);
            // Send list of users to requesting user                
            continue;
        }

        if (msg.code == "#pm")
        {
            if (cmap.find(msg.to) == cmap.end())  // User with new identifier 
            {
                message response = {msg_type::C, "#pmNO", msg.from, "Server", "User unknown"};
                msg_send(response, client);
            }
            else 
            {
                msg_send(msg, cmap[msg.to]);
                msg_send(msg, cmap[msg.from]);
            }
            continue;
        }
            
        // default behavior   
        // Broadcast text message to all users partecipating the chat 
        for (auto [k,dst] : cmap) 
        {
            // if (k != msg.from) 
            // {
                sock.sendto(buf, dst);
            // }
        }
    }

    return 0;
}