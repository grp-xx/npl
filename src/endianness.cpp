#include <cstdint>
#include <iostream>
#include <bitset>
#include <arpa/inet.h>

union Data {
    unsigned short n;
    uint8_t v[2];
};

int main()
{
    Data num = {.n = htons(42)};

    auto first = static_cast< std::bitset<8> >(num.v[0]);
    auto second = static_cast< std::bitset<8> >(num.v[1]);
    
    std::cout << "first byte: " << first << std::endl;
    std::cout << "second byte: " << second << std::endl;

    return 0;
}