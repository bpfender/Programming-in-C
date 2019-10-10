#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SKIP '0'
#define SOUNDEX_LENGTH 4

void encodeLetters(char* coded_name, char* name, int size);
void encodeSoundex(char soundex[], char* coded_name, int size);
char soundexValue(char letter);

int main(int argc, char* argv[]) {
    int length;
    char* coded_name = NULL;
    char soundex[5] = {'0', '0', '0', '0', '\0'}; /*initialise avoids 
                                                   *needing to append later 
                                                   */

    if (argc != 2) {
        printf("Error, usage is e.g. %s Pfender\n", argv[0]);
        return 1;
        /* QUESTION Where do main returns go? */
    }

    length = strlen(argv[1]);
    coded_name = (char*)malloc(length * sizeof(char));
    if (coded_name == NULL) {
        printf("Memory allocation failed...\n");
        return 1;
    }

    encodeLetters(coded_name, argv[1], length);
    encodeSoundex(soundex, coded_name, length);

    printf("%s\n", soundex);

    free(coded_name);
    return 0;
}

void encodeSoundex(char soundex[], char* coded_name, int size) {
    int i = 0, j = 0;
    int last_value = SKIP;

    while ((i < SOUNDEX_LENGTH) && (j < size)) {
        if (coded_name[j] == SKIP) {
            j++;
        } else if (coded_name[j] == last_value) {
            j++;
        } else {
            soundex[i] = coded_name[j];
            last_value = coded_name[j];
            i++;
            j++;
        }
    }
}

void encodeLetters(char* coded_name, char* name, int size) {
    int i;

    coded_name[0] = toupper(name[0]);
    for (i = 1; i < size; i++) {
        coded_name[i] = soundexValue(name[i]);
    }
}

char soundexValue(char letter) {
    switch (tolower(letter)) {
        case 'b':
        case 'f':
        case 'p':
        case 'v':
            return '1';
        case 'c':
        case 'g':
        case 'j':
        case 'k':
        case 'q':
        case 's':
        case 'x':
        case 'z':
            return '2';
        case 'd':
        case 't':
            return '3';
        case 'l':
            return '4';
        case 'm':
        case 'n':
            return '5';
        case 'r':
            return '6';
        default:
            return SKIP;
    }
}