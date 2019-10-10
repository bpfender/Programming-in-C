#include <stdio.h>
#include <stdlib.h>

typedef enum game_t { odd,
                      even,
                      number_game } game_t;

int playGame(game_t game_type, int bet);
game_t selectGame();
int playOdd(int bet);
int playEven(int bet);
int playNumbers(int bet);
int placeBet(int cash);
int spinRoulette();

int main(void) {
    game_t game_type;
    int cash = 10;
    int bet = 0;
    int winnings = 0;

    printf(
        "Welcome to the roulette machine\n\n"
        "---- You start with £%d ----\n",
        cash);

    while (cash != 0) {
        game_type = selectGame();
        bet = placeBet(cash);
        winnings = playGame(game_type, bet);

        if (winnings > 0) {
            printf("You win!!!\n");
        } else {
            printf("You lose!!\n");
        }

        cash = cash + winnings;
        printf("---- You have £%d left ----\n", cash);
    }
    return 0;
}

int playGame(game_t game_type, int bet) {
    switch (game_type) {
        case odd:
            return playOdd(bet);
        case even:
            return playEven(bet);
        case number_game:
            return playNumbers(bet);
    }
    return 0;
}

game_t selectGame() {
    int game_sel;
    int odd_even_sel;

    printf(
        "Do you want to place an odd/even bet or play a numbers game?\n"
        "Press 1 for an odd/even game...\n"
        "Press 2 for a numbers game...\n");

    scanf("%i", &game_sel);

    if (game_sel == 1) {
        printf(
            "Do you want to play for odd or even?\n"
            "Press 1 for odd...\n"
            "Press 2 for even...\n");
        scanf("%d", &odd_even_sel);

        if (odd_even_sel == 1) {
            return odd;
        } else {
            return even;
        }
    } else {
        return number_game;
    }
}

int playOdd(int bet) {
    int seed = spinRoulette();

    if (seed == 0) {
        return -bet;
    } else {
        if (seed % 2 != 0) {
            return bet;
        } else {
            return -bet;
        }
    }
}

int playEven(int bet) {
    int seed = spinRoulette();

    if (seed == 0) {
        return -bet;
    } else {
        if (seed % 2 == 0) {
            return bet;
        } else {
            return -bet;
        }
    }
}

int playNumbers(int bet) {
    int number, seed;

    printf(
        "What number do you want to bet on?\n"
        "Please enter a number between 0 and 35\n");
    scanf("%d", &number);

    seed = spinRoulette();

    if (number == seed) {
        return 34 * bet;
    } else {
        return -bet;
    }
}

int placeBet(int cash) {
    int bet;
    do {
        printf("How much do you want to bet?\n");
        scanf("%d", &bet);
        if (cash < bet) {
            printf("You don't have enough money to bet that much\n");
        }
    } while (cash < bet);
    return bet;
}

int spinRoulette() {
    int seed = rand() % 36;
    printf("The roulette has landed on %d!\n", seed);
    return seed;
}