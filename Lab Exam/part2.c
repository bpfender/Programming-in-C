#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

void passswp(char* a, char* b);

int main(void) {
    char b[100];

    passswp("ABCdefghijklmnopqrstuvwxyz", b);
    assert(strcmp(b, "@BCdefghijklmn0pqr5tuvwxyz") == 0);
    passswp("ABCdefghijklmnopqrstuvwxyz", b);
    assert(strcmp(b, "@BCdefghijklmn0pqr5tuvwxyz") == 0);

    return 0;
}

void passswp(char* a, char* b) {
    int i;

    for (i = 0; a[i] != '\0'; i++) {
        switch (tolower(a[i])) {
            case 'a':
                b[i] = '@';
                break;
            case 'o':
                b[i] = '0';
                break;
            case 's':
                b[i] = '5';
                break;
            default:
                b[i] = a[i];
                break;
        }
    }
    b[i] = '\0';
}