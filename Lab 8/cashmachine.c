#include <stdio.h>

typedef enum bool { true,
                    false } bool;

bool requestAmount(int requested_amount);
void amountsAvailable(int requested_amount);

int main(void) {
    int requested_amount;

    do {
        printf("How much money would you like ? ");
        /* QUESTION error handling for scanf? */
        scanf("%d", &requested_amount);
    } while (!requestAmount(requested_amount));

    printf("OK, dispensing ...\n");

    return 0;
}

bool requestAmount(int requested_amount) {
    if (requested_amount % 20 != 0) {
        amountsAvailable(requested_amount);
        return 0;
    }
    return 1;
}

void amountsAvailable(int requested_amount) {
    int remainder = requested_amount % 20;

    printf("I can give you %d or %d\n", requested_amount - remainder,
           requested_amount + (20 - remainder));
}