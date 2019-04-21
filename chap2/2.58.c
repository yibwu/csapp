#include <stdio.h>

int is_little_endian() 
{
    int x = 1;
    unsigned char* p = (unsigned char*)&x;
    return *p == 1 ? 1 : 0;
}

int main()
{
    int x = is_little_endian();
    printf("%d\n", x);
    return 0;
}
