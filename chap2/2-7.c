#include <stdio.h>
#include <string.h>

typedef unsigned char* byte_pointer;

void show_bytes(byte_pointer start, int len); 

int main()
{
    const char* s = "abcdef";
    show_bytes((byte_pointer)s, strlen(s));

    return 0;
}
