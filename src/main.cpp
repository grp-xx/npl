#include "socket.hpp"
#include <sys/socket.h>

int main()
{
    npl::socket<AF_INET, SOCK_STREAM> sock;

}