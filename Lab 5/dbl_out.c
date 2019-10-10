#include <stdio.h>

int main(void)
{
    char c;

    while ((c = getchar()) != EOF) {
        putchar(c);
        putchar(c);
    }
    return 0;
}