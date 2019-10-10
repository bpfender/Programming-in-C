#include <stdio.h>

int main(void)
{
    int x;
    printf("Please input a number: ");
    scanf("%i", &x);

    while(x != 1) {
        printf("%i\n",x);
        if(x%2 == 0) {
            x /= 2;
        }
        else {
            x = 3*x + 1;
        }       
    }
    printf("1\n");

    return 0;
}