#include <stdio.h>

typedef unsigned char* byte_pointer;

void show_bytes(byte_pointer start, int len);

int gen_bytes(int x, int y)
{
    x = x & (0xFF);
    y = y & (~0xFF);
    return x | y;
}

int main()
{
    int x = 0x89ABCDEF;
    int y = 0x76543210;
    int z = gen_bytes(x, y);
    show_bytes((byte_pointer)&z, sizeof(z));
    return 0;
}
