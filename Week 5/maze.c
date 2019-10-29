#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct maze {
    char** grid;
    int size_x;
    int size_y;
    int entrance_x;
    int entrance_y;
} maze;

int exploreMaze(maze* maze, int x, int y);
void findEntrance(maze* maze);
int checkExit(maze* maze);
void printMaze(maze* maze);
void test(void);

int main(void) {
    test();
    return 0;
}

int createMazeArray(maze* maze) {
    int i;

    /* TODO what the fuck! sizeof(char*)? */
    maze->grid = (char**)malloc(maze->size_y * sizeof(char*));

    if (maze->grid == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return 0;
    }

    for (i = 0; i < maze->size_y; i++) {
        maze->grid[i] = (char*)malloc(maze->size_x * sizeof(char));
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
    if (maze->grid[y][x] == 'X') {
        return 1;
    }

    maze->grid[y][x] = '#';

    if (y > 0 && maze->grid[y - 1][x] != '#') {
        if (exploreMaze(maze, x, y - 1)) {
            maze->grid[y][x] = '.';
            return 1;
        }
    }
    if (x < maze->size_x - 1 && maze->grid[y][x + 1] != '#') {
        if (exploreMaze(maze, x + 1, y)) {
            maze->grid[y][x] = '.';
            return 1;
        }
    }
    if (x > 0 && maze->grid[y][x - 1] != '#') {
        if (exploreMaze(maze, x - 1, y)) {
            maze->grid[y][x] = '.';
            return 1;
        }
    }
    if (y < maze->size_y - 1 && maze->grid[y + 1][x] != '#') {
        if (exploreMaze(maze, x, y + 1)) {
            maze->grid[y][x] = '.';
            return 1;
        }
    }

    maze->grid[y][x] = ' ';
    return 0;
}

/* TODO handling around two entrances */
void findEntrance(maze* maze) {
    int x = 0, y = 0;

    /*TODO check order of operators. Can this be rewrriten in a more foolproof way? !! */
    while (x < maze->size_x && maze->grid[0][x] != ' ') {
        x++;
    }

    /* TODO how is the maze array going to be constructed? */
    while (y < maze->size_y && maze->grid[y][0] != ' ') {
        y++;
    }

    maze->entrance_x = x;
    maze->entrance_y = y;
}

void printMaze(maze* maze) {
    int i, j;

    for (i = 0; i < maze->size_y; i++) {
        for (j = 0; j < maze->size_x; j++) {
            printf("%c", maze->grid[i][j]);
        }
        printf("\n");
    }
}

void test(void) {
    maze test_maze;

    const int grid_size = 6;

    int maze_exit;
    int i, j;

    char test_grid[6][6] = {{'#', ' ', '#', '#', '#', '#'},
                            {'#', ' ', ' ', ' ', ' ', '#'},
                            {'#', ' ', '#', '#', ' ', '#'},
                            {'#', '#', ' ', '#', ' ', '#'},
                            {'#', ' ', ' ', ' ', ' ', '#'},
                            {'#', 'X', '#', '#', '#', '#'}};

    test_maze.size_x = grid_size;
    test_maze.size_y = grid_size;

    if (!createMazeArray(&test_maze)) {
        fprintf(stderr, "TEST ERROR\n");
        exit(EXIT_FAILURE);
    }

    printf("%d %d\n", test_maze.size_x, test_maze.size_y);

    for (i = 0; i < grid_size; i++) {
        for (j = 0; j < grid_size; j++) {
            test_maze.grid[i][j] = test_grid[i][j];
        }
    }

    printMaze(&test_maze);

    findEntrance(&test_maze);
    printf("x: %d y: %d\n", test_maze.entrance_x, test_maze.entrance_y);

    maze_exit = exploreMaze(&test_maze, 0, 1);
    printf("Exit status: %d\n", maze_exit);

    printMaze(&test_maze);
}
