#include <stdio.h>
#include <math.h>

int main(void)
{
    double x;
    int result;

    printf("sin^2(x) + cos^2(x) = 1\n");
    printf("Please type in a number to demonstrate this: ");
    scanf("%lf", &x);

    result = (int) (cos(x)*cos(x) + sin(x)*sin(x));

    printf("The result is %i\n", result);

    return 0;
}