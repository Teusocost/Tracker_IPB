#include <stdio.h>

int main()
{
    float i = 4.245252145634;
    printf("\n%.12f", i);
    i = i, 4;
    printf("\n%.12f", i);

    return 0;
}