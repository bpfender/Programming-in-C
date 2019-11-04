#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct maze {
    char** final;
    int min_route;
    char** test;
    int size_x;
    int size_y;
    int entrance_x;
    int entrance_y;
} maze;

void exploreMaze(maze* maze, int x, int y, int length);
void savePath(maze* maze, int length);
void findEntrance(maze* maze);
int isExit(maze* maze, int x, int y);

int loadMaze(char* filename, maze* maze);
int readMazeSize(maze* maze, char* line);
int initialiseMaze(maze* maze);
char** createMazeArray(int size_x, int size_y);
void freeMazeArray(maze* maze);

int getLine(char** buffer, int* size, FILE* file);
void printSolution(maze* maze);

void test(void);

int main(int argc, char* argv[]) {
    maze maze;
    /* test();*/

    if (argc != 2) {
        fprintf(stderr,
                "ERROR: Incorrect usage, try e.g. %s maze.txt\n",
                argv[0]);
        return 1;
    }

    if (!loadMaze(argv[1], &maze)) {
        return 1;
    }

    findEntrance(&maze);
    exploreMaze(&maze, maze.entrance_x, maze.entrance_y, 0);

    if (maze.min_route == -1) {
        printf("No route through maze\n");
    } else {
        printf("Shortest route is %d steps\n", maze.min_route);
        printSolution(&maze);
    }

    freeMazeArray(&maze);
    return 0;
}

void exploreMaze(maze* maze, int x, int y, int length) {
    maze->test[y][x] = '.';

    if (isExit(maze, x, y)) {
        savePath(maze, length);
        return;
    }

    if (y > 0 && maze->test[y - 1][x] == ' ') {
        exploreMaze(maze, x, y - 1, length + 1);
        maze->test[y - 1][x] = ' ';
    }
    if (x < maze->size_x - 1 && maze->test[y][x + 1] == ' ') {
        exploreMaze(maze, x + 1, y, length + 1);
        maze->test[y][x + 1] = ' ';
    }
    if (x > 0 && maze->test[y][x - 1] == ' ') {
        exploreMaze(maze, x - 1, y, length + 1);
        maze->test[y][x - 1] = ' ';
    }
    if (y < maze->size_y - 1 && maze->test[y + 1][x] == ' ') {
        exploreMaze(maze, x, y + 1, length + 1);
        maze->test[y + 1][x] = ' ';
    }
}

void savePath(maze* maze, int length) {
    int i, j;

    if (maze->min_route < 0) {
        maze->min_route = length;

    } else if (length < maze->min_route) {
        maze->min_route = length;
    }

    /* Copy current shortest path into solution array */
    for (i = 0; i < maze->size_y; i++) {
        for (j = 0; j < maze->size_y; j++) {
            maze->final[i][j] = maze->test[i][j];
        }
    }
}

void findEntrance(maze* maze) {
    int x = 0, y = 0;

    while (x < maze->size_x && maze->test[0][x] != ' ') {
        x++;
    }

    while (y < maze->size_y && maze->test[y][0] != ' ') {
        y++;
    }

    if (x < y) {
        maze->entrance_x = x;
        maze->entrance_y = 0;
    } else {
        maze->entrance_x = 0;
        maze->entrance_y = y;
    }
}

int isExit(maze* maze, int x, int y) {
    if (y == 0 || y == maze->size_y - 1 ||
        x == 0 || x == maze->size_x - 1) {
        if (!(y == maze->entrance_y && x == maze->entrance_x)) {
            return 1;
        }
    }
    return 0;
}

int loadMaze(char* filename, maze* maze) {
    FILE* file;
    char* buffer = NULL;
    int size;

    int len;
    int line = 0;

    int i;

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return 0;
    }
    getLine(&buffer, &size, file);

    if (!readMazeSize(maze, buffer)) {
        return 0;
    }

    if (!initialiseMaze(maze)) {
        return 0;
    }

    while ((len = getLine(&buffer, &size, file))) {
        if (len != maze->size_x) {
            fprintf(stderr, "Invalid line length in line %d\n", line + 1);
            return 0;
        }
        for (i = 0; buffer[i] != '\0'; i++) {
            maze->test[line][i] = buffer[i];
            maze->final[line][i] = buffer[i];
        }

        if (line > maze->size_y - 1) {
            fprintf(stderr, "Too many lines in file\n");
            return 0;
        }
        line++;
    }
    if (line != maze->size_y) {
        fprintf(stderr, "Too few lines in file\n");
        return 0;
    }

    free(buffer);
    fclose(file);

    return 1;
}

int readMazeSize(maze* maze, char* line) {
    if (sscanf(line, "( %d , %d )", &maze->size_x, &maze->size_y) == 2) {
        return 1;
    }

    fprintf(stderr, "Grid size input error\n");
    return 0;
}

int initialiseMaze(maze* maze) {
    maze->final = createMazeArray(maze->size_x, maze->size_y);
    if (maze->final == NULL) {
        return 0;
    }

    maze->test = createMazeArray(maze->size_x, maze->size_y);
    if (maze->test == NULL) {
        return 0;
    }

    maze->min_route = -1;

    return 1;
}

char** createMazeArray(int size_x, int size_y) {
    int i;
    char** grid;

    grid = (char**)malloc(size_y * sizeof(char*));

    if (grid == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    for (i = 0; i < size_y; i++) {
        grid[i] = (char*)malloc(size_x * sizeof(char));

        if (grid[i] == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            return NULL;
        }
    }
    return grid;
}

void freeMazeArray(maze* maze) {
    int i;

    for (i = 0; i < maze->size_y; i++) {
        free(maze->test[i]);
        free(maze->final[i]);
    }
    free(maze->test);
    free(maze->final);
}

/* MODIFIED TO RETURN NULL CHARACTER TERMINATED LINE \n removed*/
int getLine(char** buffer, int* size, FILE* file) {
    const int factor = 2;
    int file_pos = (int)ftell(file);
    int i = 0;

    /* FIXME 20 magic number */
    if (*buffer == NULL) {
        *size = 5;
        *buffer = (char*)malloc((*size + 1) * sizeof(char));
    }
    if (*buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    while (fgets(*buffer + i, *size - i, file)) {
        /* Additional i-1 for index, because file pos will be one higher after reading \n */
        /* TODO some clearer coding around returning i would be preferred */
        i = ftell(file) - file_pos;
        if ((*buffer)[i - 1] == '\n') {
            (*buffer)[i - 1] = '\0';
            return i - 1;
        }

        if (!(i < *size - 1)) {
            *size *= factor;
            /* TODO, can this be nicer? */
            *buffer = realloc(*buffer, *size * sizeof(char));
            if (*buffer == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return -1;
            }
        }
    }

    if (!feof(file)) {
        fprintf(stderr, "Error reading file\n");
        return -1;
    }
    return i;
}

void printSolution(maze* maze) {
    int i, j;

    for (i = 0; i < maze->size_y; i++) {
        for (j = 0; j < maze->size_x; j++) {
            printf("%c", maze->final[i][j]);
        }
        printf("\n");
    }
}

void test(void) {
    maze test_maze;

    const int test_size = 6;

    int i, j;

    char test_test[6][6] = {{'#', ' ', '#', '#', '#', '#'},
                            {'#', ' ', ' ', ' ', ' ', '#'},
                            {'#', '#', '#', '#', ' ', '#'},
                            {'#', ' ', ' ', '#', ' ', '#'},
                            {'#', ' ', ' ', ' ', ' ', '#'},
                            {'#', ' ', '#', '#', '#', '#'}};

    test_maze.size_x = test_size;
    test_maze.size_y = test_size;

    if (!initialiseMaze(&test_maze)) {
        fprintf(stderr, "TEST ERROR\n");
        exit(EXIT_FAILURE);
    }

    printf("%d %d\n", test_maze.size_x, test_maze.size_y);

    for (i = 0; i < test_size; i++) {
        for (j = 0; j < test_size; j++) {
            test_maze.test[i][j] = test_test[i][j];
            test_maze.final[i][j] = test_test[i][j];
        }
    }

    printSolution(&test_maze);

    findEntrance(&test_maze);
    printf("Entrance x: %d y: %d\n", test_maze.entrance_x, test_maze.entrance_y);

    exploreMaze(&test_maze, 1, 0, 0);
    printf("Check exit: %d\n", isExit(&test_maze, 1, 0));
    printf("Check exit: %d\n", isExit(&test_maze, 1, 5));

    printSolution(&test_maze);

    readMazeSize(&test_maze, "(15 ,20)");
    printf("sizex: %d sizey: %d\n", test_maze.size_x, test_maze.size_y);

    printf("Route length %d\n", test_maze.min_route);

    printSolution(&test_maze);

    loadMaze("./Text Files/maze.txt", &test_maze);

    freeMazeArray(&test_maze);
}
