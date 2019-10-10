#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int median = RAND_MAX/2;
    int i;
    int minus_cnt = 0, plus_cnt = 0;

    for (i = 0; i < 500; i++ ) {
        if (rand() < median) {
            plus_cnt ++;
        } else {
            minus_cnt ++;
        }
        printf("%i\n", plus_cnt - minus_cnt);
    }


    return 0;
}