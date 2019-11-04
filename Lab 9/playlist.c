#include <stdio.h>
#include <stdlib.h>

void FisherYatesShuffle(int* list, int size);
int randomIndex(int size);
void swap(int* to_swap, int* list_end);
int createList(int** list);
void initialiseList(int* list, int size);
void printShuffled(int* list, int size);

int main(void) {
    int* list = NULL;
    int size = 0;

    size = createList(&list);
    initialiseList(list, size);
    FisherYatesShuffle(list, size);
    printShuffled(list, size);

    free(list);
    return 0;
}

/* QUESTION double pointer notation */
int createList(int** list) {
    int size;
    printf("How many songs required ? ");
    if (!scanf("%d", &size)) {
        printf("Incorrect input. Aborting...\n");
        abort();
    }

    *list = (int*)malloc(size * sizeof(int));
    if (*list == NULL) {
        printf("Memory allocation failed...\n");
        abort();
    }
    return size;
}

void FisherYatesShuffle(int* list, int size) {
    while (size - 1) {
        swap(&list[randomIndex(size)], &list[size - 1]);
        size--;
    }
}

int randomIndex(int size) {
    return rand() % size;
}

void swap(int* to_swap, int* list_end) {
    int tmp;
    tmp = *list_end;
    *list_end = *to_swap;
    *to_swap = tmp;
}

void initialiseList(int* list, int size) {
    int i;
    /* QUESTION size_t type */
    for (i = 0; i < size; i++) {
        list[i] = i + 1;
    }
}

void printShuffled(int* list, int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("%d ", list[i]);
    }
    printf("\n");
}