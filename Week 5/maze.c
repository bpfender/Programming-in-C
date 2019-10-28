#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct maze {
    char* layout;
    char** grid;
    int size_x;
    int size_y;
    int entrance_x;
    int entrance_y;
} maze;

int exploreMaze(maze* maze, int x, int y);
void findEntrance(maze* maze);
int checkExit(maze* maze);
void test(void);

int main(void) {
    test();
    return 0;
}

int createMazeArray(maze* maze) {
    int i;

    maze->grid = (int**)malloc(maze->size_y * sizeof(char));

    if (maze->grid == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return 0;
    }

    for (i = 0; i < maze->size_y; i++) {
        maze->grid[i] = (int*)malloc(maze->size_x * sizeof(char));
        if (maze->grid[i] == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            return 0;
        }
    }

    return 1;
}

void freeMazeArray(maze* maze) {
    int i;

    for (i = 0; i < maze->size_y; i++) {
        free(maze->grid[i]);
    }
    free(maze->grid);
}

int exploreMaze(maze* maze, int x, int y) {
    printf("x:%d y:%d\n", x, y);

    /* TODO how best to mark exit from maze? */
    if (maze->layout[x + y * maze->size_x] == 'X') {
        return 1;
    }

    maze->layout[x + y * maze->size_x] = '#';

    if (y > 0 && maze->layout[x + (y - 1) * maze->size_x] != '#') {
        if (exploreMaze(maze, x, y - 1)) {
            return 1;
        }
    }
    if (x < maze->size_x - 1 && maze->layout[x + 1 + y * maze->size_x] != '#') {
        if (exploreMaze(maze, x + 1, y)) {
            return 1;
        }
    }
    if (x > 0 && maze->layout[x - 1 + y * maze->size_x] != '#') {
        if (exploreMaze(maze, x - 1, y)) {
            return 1;
        }
    }
    if (y < maze->size_y - 1 && maze->layout[x + (y + 1) * maze->size_x] != '#') {
        if (exploreMaze(maze, x, y + 1)) {
            return 1;
        }
    }
    return 0;
}

/* TODO handling around two entrances */
void findEntrance(maze* maze) {
    int x = 0, y = 0;
    while (maze->layout[x] != ' ' && x < maze->size_x) {
        x++;
    }

    /* TODO how is the maze array going to be constructed? */
    while (maze->layout[y * maze->size_x] != ' ' && y < maze->size_y) {
        y++;
    }

    maze->entrance_x = x;
    maze->entrance_y = y;
}

void test(void) {
    maze test_maze;
    int maze_exit;

    char test_array[16] = {'#', ' ', '#', '#',
                           '#', ' ', '#', '#',
                           '#', ' ', ' ', '#',
                           '#', 'X', '#', '#'};

    test_maze.size_x = 4;
    test_maze.size_y = 4;

    test_maze.layout = &test_array;

    findEntrance(&test_maze);
    printf("x: %d y: %d\n", test_maze.entrance_x, test_maze.entrance_y);

    maze_exit = exploreMaze(&test_maze, 1, 0);
    printf("Exit status: %d\n", maze_exit);
}
