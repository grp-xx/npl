#include <cstdint>
#include <iostream>

union endy {
    uint16_t num;
    uint8_t  c[2];
};

int main()
{
    endy m;
    m.num = 1042;
    std::cout << "H: " << static_cast<unsigned short>(m.c[0]) << "   " << "L: " << static_cast<unsigned short>(m.c[1]) << std::endl;

}