#include <stdio.h>

typedef unsigned char* byte_pointer;

void show_bytes(byte_pointer start, int len);

unsigned replace_byte(unsigned x, unsigned char b, int i)
{
    int shift = i << 3; 

    x = x & ~(0xFF << shift);
    return x | (b << shift);
}

int main()
{
    unsigned x = replace_byte(0x12345678, 0xAB, 2);
    unsigned y = replace_byte(0x12345678, 0xAB, 0);
    show_bytes((byte_pointer)&x, sizeof(x));
    show_bytes((byte_pointer)&y, sizeof(y));
    return 0;
}
