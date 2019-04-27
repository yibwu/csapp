#include <stdio.h>

int int_shifts_are_logical()
{
    int x = -1;
    return x == x >> 1 ? 1 : 0;
}

int main()
{
    printf("%d\n", int_shifts_are_logical());

    return 0;
}
