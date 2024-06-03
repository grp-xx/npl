#include <iostream>
#include "./20240603.hpp"



int main()
{

    tcpflowid_t f(1,2,3,4), g(3,4,1,2), h(1,2,3,4), w(5,6,7,8);

    std::cout << "std::equal_to: " << ( std::equal_to<tcpflowid_t>()(f,h)? "Correct" :   "Wrong" ) << std::endl;
    std::cout << "std::equal_to: " << ( std::equal_to<tcpflowid_t>()(f,g)? "Correct" :   "Wrong" ) << std::endl;
    std::cout << "std::equal_to: " << ( std::equal_to<tcpflowid_t>()(f,w)? "Wrong"   : "Correct" ) << std::endl;
    std::cout << "    std::hash: " << ( ( std::hash<tcpflowid_t>()(f) >= 0 )? "Correct" : "Wrong" ) << std::endl;
    std::cout << "    std::hash: " << ( ( std::hash<tcpflowid_t>()(f) == std::hash<tcpflowid_t>()(h) )? "Correct" : "Wrong" ) << std::endl;
    std::cout << "    std::hash: " << ( ( std::hash<tcpflowid_t>()(f) != std::hash<tcpflowid_t>()(w) )? "Correct" : "Wrong" ) << std::endl;

    return EXIT_SUCCESS;

}