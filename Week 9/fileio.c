#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 5
#define BUFF_FACT 4

FILE* openFile(char* filename);
size_t getLine(char** buffer, size_t* size, FILE* file);

/*void test(void);

int main(void) {
    test();
    return 0;
}*/

FILE* openFile(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return NULL;
    }
    return file;
}

/* Reads line from a file. '\n' is appended by '\0' character. Must be passed
 * either initialised buffer or NULL pointer. Returns 0 when EOF reached or
 * error in file reading. This should be checked after return from getLine().
 * Returns number of characters in string including \n but not \0
 */
size_t getLine(char** buffer, size_t* size, FILE* file) {
    size_t i = 0;
    long int file_pos = ftell(file);
    char* tmp;

    /* If buffer has not been initialised, buffer is malloced and size is set */
    if (!*buffer) {
        *size = BUFF_SIZE;
        *buffer = (char*)malloc(BUFF_SIZE * sizeof(char));
        if (!*buffer) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Use fgets to read into buffer. Space remaining in buffer is decremnted 
       based on how far into the file has been read. Allows for dyanmic resizing
       of buffer. The while loop handles \n characters. If EOF is reached, while
       loop will exit */
    while (fgets(*buffer + i, *size - i, file)) {
        /* Get number of characters read */
        i = ftell(file) - file_pos;

        /* Check whether last read character was a \n or eof reached*/
        if ((*buffer)[i - 1] == '\n' || feof(file)) {
            return i;
        }
        /* Check that buffer hasn't been filled. If it has expand, so rest of
           of the line can be read in */
        if (!(i < *size - 1)) {
            *size *= BUFF_FACT;
            tmp = realloc(*buffer, *size * sizeof(char));
            if (!*tmp) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            *buffer = tmp;
        }
    }

    /* Returns 0 on eof or error */
    return 0;
}

/*void test(void) {
    FILE* file = openFile("./test.txt");
    char* buffer = NULL;
    size_t size;
    size_t len;

    while ((len = getLine(&buffer, &size, file))) {
        printf("%li\n", len);
        printf("%s\n", buffer);
    }

    len = getLine(&buffer, &size, file);
    printf("%li\n", len);
    printf("%s\n", buffer);
}*/