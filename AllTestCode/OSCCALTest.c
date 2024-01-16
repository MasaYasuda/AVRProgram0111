#define F_CPU 1000000L
#include <avr/io.h>

int main()
{
    OSCCAL = 0xAD; // クロック校正(値を調整)
    while (1)
    {
    }
}
