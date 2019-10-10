#include <stdio.h>

int main(void)
{
    int a, b, c;

    printf("Input three integers: ");
    
    scanf("%i %i %i", &a, &b, &c) == 0;
    /*TODO no error checking at all*/

    if(a==2 && b==3 && c==7){
        printf("Twice the sum of integers plus 7 is %i\n", (a+b+c)*2+7);
    }

    return 0;
}