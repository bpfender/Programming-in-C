#define NDEBUG
#include <assert.h>
#include <stdio.h>

void test(void);
int buttonPresses(const int time_options[], int time_remaining);

int main(void) {
    const int time_options[] = {600, 60, 10};
    int cook_time;

    test();

    printf("Type the time required\n");
    if (!scanf("%d", &cook_time)) {
        printf("Incorrect input. Aborting...\n");
        return 1;
    }

    printf("Number of button presses = %d\n", buttonPresses(time_options, cook_time));

    return 0;
}

int buttonPresses(const int time_options[], int time_remaining) {
    int i;

    for (i = 0; i < 3; i++) {
        if (time_remaining > time_options[i]) {
            time_remaining -= time_options[i];
            return 1 + buttonPresses(time_options, time_remaining);
        }
    }
    return 1;
}

void test(void) {
    int time_options[] = {600, 60, 10};
    assert(buttonPresses(time_options, 25) == 3);
    assert(buttonPresses(time_options, 705) == 7);
}