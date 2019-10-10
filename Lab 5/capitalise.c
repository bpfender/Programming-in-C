#include <stdio.h>
#include <ctype.h>

#define CAPS ('A' - 'a')

int main(void)
{
    int c;

    while ((c = getchar()) != EOF) {
        if (islower(c)) {
            putchar(toupper(c));
        }
        else {
            putchar(c);
        }
    }
    return 0;
}