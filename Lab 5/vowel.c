#include <stdio.h>
#include <ctype.h>

int isvowel(int c);

int main(void)
{
    int c;

    while ((c = getchar()) != EOF)
    {
        if (isvowel(c))
        {
            if (islower(c))
            {
                putchar(toupper(c));
            }
            else
            {
                putchar(c);
            }
        }
        else
        {
            if (isupper(c))
            {
                putchar(tolower(c));
            }
            else
            {
                putchar(c);
            }  
        }
    }
    return 0;
}

int isvowel(int c)
{
    switch (c) {
        case 'A':
        case 'E':
        case 'I':
        case 'O':
        case 'U':
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
            return 1;
        default:
            return 0;
    }
}
