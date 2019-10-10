#include <stdio.h>

#define HEIGHT 17
#define HEIGHT_2 3

int main(void)
{
    int i = 0;
    
    printf("\n\nIIIIIII\n");
    while (i < HEIGHT) {
        printf("  III\n");
        i = i + 1;
    }
    printf("IIIIIII\n\n\n");
 
  
    printf("IIIIIII\n");
    for(i = 0; i < HEIGHT_2; i++)
    {
        printf("II\n");
    }
    printf("IIIIIII\n");

    return 0;
}

