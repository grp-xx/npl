#include <cstdlib>
#include <curses.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <ncurses.h>
#include "json.hpp"
#include "socket.hpp"
#include "sockaddress.hpp"
#include "chatroom.hpp"

npl::socket<AF_INET, SOCK_DGRAM> sock;
std::mutex m;
int rows, cols;

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

void transmitter(const npl::sockaddress<AF_INET>& srv_addr, const std::string& user, WINDOW* win)
{

    nodelay(win, true);

    for (;;) 
    {
        m.lock();
        wprintw(win, "<You>: ");
        refresh();
        m.unlock();
        std::string line = "";
        char c;
        do 
        {
            m.lock();
            c = wgetch(win);
            m.unlock();
            if (c != ERR) line += c;  
        }
        while (c != '\n');
        m.lock();
        wclear(win);
        m.unlock();

        // wprintw(win, line.c_str());

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

void receiver(const std::string& user, WINDOW* win)
{
    for(;;)
    {
        message msg = msg_rcv();

        std::string prompt = (msg.code == "#pm" ? msg.from + " --> " + msg.to : msg.from);
        std::stringstream ss;
        ss << "<" << prompt << ">" << ": " << msg.text << std::endl;
        m.lock();
        wprintw(win, "%s", ss.str().c_str());
        wrefresh(win);
        m.unlock();
        if (msg.code == "#byeOK") {
            exit(0);
        }
    
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

    // Initilaize ncurses screen

    initscr();
    refresh();

    getmaxyx(stdscr, rows, cols);
    attron(A_BOLD);
    mvprintw(0, 1, "Messages");
    attroff(A_BOLD);
    refresh();
    curs_set(0);
    WINDOW *outframe = newwin(rows-4, cols-1,1,0);
    wborder(outframe,0,0,0,0,0,0,0,0);
    wrefresh(outframe);
    WINDOW *msgwin = newwin(rows-6, cols-3, 2, 1);
    wrefresh(msgwin);
    scrollok(msgwin, true);

    attron(A_BOLD);
    mvprintw(rows-2, 1, "%s", user.c_str()); // Print username on top of input win
    attroff(A_BOLD);
    refresh();
    WINDOW *inframe = newwin(3, cols-1,rows-4,0);
    wborder(outframe,0,0,0,0,0,0,0,0);
    wrefresh(outframe);
    WINDOW *inwin = newwin(1, cols-3, rows-2, 1);
    wrefresh(inwin);
    scrollok(inwin, true);

    std::thread rx(receiver, user, msgwin);
    std::thread tx(transmitter, srv_addr, user, inwin);

    rx.join();
    tx.join();

    endwin();

    return EXIT_SUCCESS;
}
