#include <stdio.h>

int even_ones(unsigned x)
{
    unsigned y = 0xAAAAAAAA;
    x &= y;
    return x != 0 && (x & (x - 1)) == 0;
}

int main()
{
    unsigned arr[] = {1, 2, 5, 8, 10, 15, 32};
    int count = sizeof(arr) / sizeof(unsigned);
    for (int i = 0; i < count; i++) 
    {
        printf("input: %u, output: %d\n", arr[i], even_ones(arr[i]));
    }

    return 0;
}
