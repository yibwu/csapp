/*
A. !(x ^ ~0) 
B. !(x ^ 0) 
C. !((x & (0xFF << ((sizeof(int) - 1) << 3))) ^ (0xFF << ((sizeof(int) - 1) << 3)))
D. !(x & 0xFF) 
*/

#include <stdio.h>

int test1(int x)
{
    return !(x ^ ~0);
}

int test2(int x)
{
    return !(x ^ 0);
}

int test3(int x)
{
    return !((x & (0xFF << ((sizeof(int) - 1) << 3))) ^ (0xFF << ((sizeof(int) - 1) << 3)));
}

int test4(int x)
{
    return !(x & 0xFF);
}

int main()
{
    printf("%d\n", test1(~0)); // expect 1
    printf("%d\n", test1(1)); // expect 0

    printf("%d\n", test2(0)); // expect 1
    printf("%d\n", test2(1)); // expect 0

    printf("%d\n", test3(0xFF << 24)); // expect 1
    printf("%d\n", test3(0xFF << 23)); // expect 0

    printf("%d\n", test4(0xFF << 8)); // expect 1
    printf("%d\n", test4(0xFF << 7)); // expect 0

    return 0;
}
