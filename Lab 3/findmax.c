#include <stdio.h>

int main(void)
{
    int i, num;
    float x, max;
   
    printf("How many numbers do you wish to enter? ");
    scanf("%i",&num);
    
    printf("\nEnter %i real numbers: ", num);
    

    scanf("%f", &max);
    for(i = 0; i < num-1; i++) {
        scanf("%f", &x);
        x > max ? max = x: max;
    }

    printf("Maximum value: %f\n", max);

    return 0;
}