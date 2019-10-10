#include <stdio.h>

int main(void) {
    int hour1, min1, hour2, min2;
    int time_diff_min;
    int hourdiff, mindiff;

    printf("Enter two times: ");

    if (scanf("%i:%i %i:%i", &hour1, &min1, &hour2, &min2) != 4) {
        printf("Invalid input\n");
        return 1;
    }

    time_diff_min = (60 * hour2 + min2) - (60 * hour1 + min1);

    if (time_diff_min < 0) {
        time_diff_min = 60 * 24 + time_diff_min;
    }

    hourdiff = time_diff_min / 60;
    mindiff = time_diff_min % 60;

    printf("Difference is: %02i:%02i\n", hourdiff, mindiff);

    return 0;
}
