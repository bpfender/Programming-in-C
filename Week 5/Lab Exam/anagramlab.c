#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ALPHABET 26

int anagram(char s1[], char s2[]);

int main(void) {
    assert(anagram("elvis", "lives") == 0);
    assert(anagram("dreads", "sadder") == 1);
    assert(anagram("replays", "parsley") == 1);
    assert(anagram("listen", "silent") == 0);
    assert(anagram("orchestra", "carthorse") == 1);

    /* Two identical words are not anagrams */
    assert(anagram("elvis", "elvis") == 0);

    assert(anagram("neill", "neill") == 0);
    assert(anagram("neil", "neill") == 0);
    assert(anagram("horse", "short") == 0);

    return 0;
}

int anagram(char s1[], char s2[]) {
    int i;

    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int counts[ALPHABET] = {0};

    if (len1 != len2 || strcmp(s1, s2) == 0) {
        return 0;
    }

    for (i = 0; s1[i] != '\0'; i++) {
        if (s1[i] == s2[i]) {
            return 0;
        }
        counts[tolower(s1[i]) - 'a']++;
        counts[tolower(s2[i]) - 'a']--;
    }

    for (i = 0; i < ALPHABET; i++) {
        if (counts[i]) {
            return 0;
        }
    }
    return 1;
}