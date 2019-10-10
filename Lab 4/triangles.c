#include <stdio.h>

void right_angled(int a, int b, int c);

int main(void)
{
    int a, b, c;

    while (a != -999)
    {
        printf("Please input three integer values: ");
        scanf("%i%i%i", &a, &b, &c);

        if( a == -999)
            printf("Exiting...");
        else if (a <= 0 || b <= 0 || c <= 0)
            printf("Invalid input\n");
        else if (a == b && b == c)
            printf("Equilateral\n");
        else if (a == b || b == c || a == c)
            printf("Iscosceles\n");
        else
            printf("Scalene\n");

        right_angled(a, b, c);
    }
    return 0;
}

void right_angled(int a, int b, int c)
{
    a = a*a;
    b = b*b;
    c = c*c;

    if( c == a+b || b == a+c || c == a+b)
        printf("RA\n");
    else
        printf("Not RA\n");
}