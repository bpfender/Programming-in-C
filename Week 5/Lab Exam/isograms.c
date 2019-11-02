#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ALPHABET 26

int isogram(char* s);

int main(void) {
    assert(isogram("programming") == 0);
    assert(isogram("housewarmings") == 0);
    assert(isogram("abductions") == 1);
    assert(isogram("housewarming") == 1);
    assert(isogram("hydromagnetics") == 1);
    assert(isogram("uncopyrightable") == 1);
    return 0;
}

int isogram(char* s) {
    int i;
    int counts[ALPHABET] = {0};

    for (i = 0; s[i] != '\0'; i++) {
        counts[tolower(s[i]) - 'a']++;
    }

    for (i = 0; i < ALPHABET; i++) {
        if (counts[i] > 1) {
            return 0;
        }
    }
    return 1;
}
