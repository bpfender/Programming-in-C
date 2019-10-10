#include <stdio.h>

int main(void)
{
    int i, num;
    int x, max, max_flag = 0;
   
    printf("How many numbers do you wish to enter? ");
    scanf("%i",&num);
    
    printf("Enter %i integers (only odd ones will be analysed): ", num);
        
    for(i = 0; i < num; i++) {
        scanf("%i", &x);
        if (x % 2 != 0) {
            if (max_flag == 0) {
                max = x;
                max_flag = 1;
            }
            else {
            x > max ? max = x: max;
            }
        }
    }

    if(max_flag) {
        printf("Maximum value: %i\n", max);
    }
    else {
        printf("No odd numbers entered\n");
    }

    return 0;
}