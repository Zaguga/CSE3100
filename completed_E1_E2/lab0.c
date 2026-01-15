#include <stdio.h>

int main(void)
{
    int i, sum;
    i = 0;
    sum = 0;

    printf("Hello, World!\n");
    while (i < 200)
    {
        sum = sum + i;
        i = i + 2;
    }
    printf("%d\n", sum);

    return 0;
}