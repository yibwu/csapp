#include <stdio.h>

int any_even_one(unsigned x)
{
    unsigned y = 0xAAAAAAAA;
    return (x & y) != 0;
}

int main()
{
    printf("%d\n", any_even_one(1));
    printf("%d\n", any_even_one(2));
    printf("%d\n", any_even_one(5));
    printf("%d\n", any_even_one(8));

    return 0;
}
