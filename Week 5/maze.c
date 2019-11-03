#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct maze {
    char** final_grid;
    int route_length;
    char** grid;
    int size_x;
    int size_y;
    int entrance_x;
    int entrance_y;
} maze;

int exploreMaze(maze* maze, int x, int y, int length);
int createMazeArray(maze* maze);
void freeMazeArray(maze* maze);
int isExit(maze* maze, int x, int y);
int loadMaze(char* filename, maze* maze);
int readMazeSize(maze* maze, char* line);
int getLine(char** buffer, int* size, FILE* file);
void findEntrance(maze* maze);
int checkExit(maze* maze);
void printMaze(maze* maze);
void test(void);

int main(int argc, char* argv[]) {
    maze maze;
    int i, j;

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
    printMaze(&maze);
    findEntrance(&maze);

    exploreMaze(&maze, maze.entrance_x, maze.entrance_y, 0);

    for (i = 0; i < maze.size_y; i++) {
        for (j = 0; j < maze.size_x; j++) {
            printf("%c", maze.final_grid[i][j]);
        }
        printf("\n");
    }

    freeMazeArray(&maze);
    return 0;
}

int createMazeArray(maze* maze) {
    int i;

    maze->grid = (char**)malloc(maze->size_y * sizeof(char*));
    maze->final_grid = (char**)malloc(maze->size_y * sizeof(char*));

    if (maze->grid == NULL || maze->final_grid == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return 0;
    }

    for (i = 0; i < maze->size_y; i++) {
        maze->grid[i] = (char*)malloc(maze->size_x * sizeof(char));
        maze->final_grid[i] = (char*)malloc(maze->size_x * sizeof(char));
        if (maze->grid[i] == NULL || maze->final_grid[i] == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            return 0;
        }
    }

    maze->route_length = -1;

    return 1;
}

void freeMazeArray(maze* maze) {
    int i;

    for (i = 0; i < maze->size_y; i++) {
        free(maze->grid[i]);
        free(maze->final_grid[i]);
    }
    free(maze->grid);
    free(maze->final_grid);
}

int isExit(maze* maze, int x, int y) {
    if (y == 0 || y == maze->size_y - 1 || x == 0 || x == maze->size_x - 1) {
        if (!(y == maze->entrance_y && x == maze->entrance_x)) {
            return 1;
        }
    }
    return 0;
}

int exploreMaze(maze* maze, int x, int y, int length) {
    int i, j;
    printf("Cood x:%d y:%d\n", x, y);
    maze->grid[y][x] = '.';

    if (isExit(maze, x, y)) {
        printf("Length %d x %d y %d\n", length, x, y);
        if (maze->route_length < 0) {
            maze->route_length = length;

        } else if (length < maze->route_length) {
            maze->route_length = length;
        }

        for (i = 0; i < maze->size_y; i++) {
            for (j = 0; j < maze->size_y; j++) {
                maze->final_grid[i][j] = maze->grid[i][j];
            }
        }

        return 1;
    }

    if (y > 0 && maze->grid[y - 1][x] == ' ') {
        exploreMaze(maze, x, y - 1, length + 1);
        maze->grid[y - 1][x] = ' ';
    }
    if (x < maze->size_x - 1 && maze->grid[y][x + 1] == ' ') {
        exploreMaze(maze, x + 1, y, length + 1);
        maze->grid[y][x + 1] = ' ';
    }
    if (x > 0 && maze->grid[y][x - 1] == ' ') {
        exploreMaze(maze, x - 1, y, length + 1);
        maze->grid[y][x - 1] = ' ';
    }
    if (y < maze->size_y - 1 && maze->grid[y + 1][x] == ' ') {
        exploreMaze(maze, x, y + 1, length + 1);
        maze->grid[y + 1][x] = ' ';
    }
    return 1; /*maze->grid[y][x] = ' ';*/
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

    if (x < y) {
        maze->entrance_x = x;
        maze->entrance_y = 0;
    } else {
        maze->entrance_x = 0;
        maze->entrance_y = y;
    }
}

int loadMaze(char* filename, maze* maze) {
    FILE* file;
    char* buffer = NULL;
    int buff_size;
    int line_len;
    int line_index = 0;
    int i;

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return 0;
    }
    getLine(&buffer, &buff_size, file);

    if (!readMazeSize(maze, buffer)) {
        return 0;
    }

    if (!createMazeArray(maze)) {
        return 0;
    }

    while ((line_len = getLine(&buffer, &buff_size, file))) {
        if (line_len != maze->size_x) {
            fprintf(stderr, "Invalid line length in line %d\n", line_index + 1);
            return 0;
        }
        for (i = 0; buffer[i] != '\0'; i++) {
            maze->grid[line_index][i] = buffer[i];
        }

        if (line_index > maze->size_y - 1) {
            printf("%d ", line_index);
            fprintf(stderr, "Too many lines in file\n");
            return 0;
        }
        line_index++;
    }

    free(buffer);
    fclose(file);

    return 1;
}

int readMazeSize(maze* maze, char* line) {
    /* TODO what could go wrong with this? Also can we talk bmore about string literals */
    if (sscanf(line, "( %d , %d )", &maze->size_x, &maze->size_y) == 2) {
        return 1;
    }

    fprintf(stderr, "Grid size input error\n");
    return 0;
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

void test(void) {
    maze test_maze;

    const int grid_size = 6;

    int maze_exit;
    int i, j;

    char test_grid[6][6] = {{'#', ' ', '#', '#', '#', '#'},
                            {'#', ' ', ' ', ' ', ' ', '#'},
                            {'#', '#', '#', '#', ' ', '#'},
                            {'#', ' ', ' ', '#', ' ', '#'},
                            {'#', ' ', ' ', ' ', ' ', '#'},
                            {'#', ' ', '#', '#', '#', '#'}};

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
            test_maze.final_grid[i][j] = test_grid[i][j];
        }
    }

    printMaze(&test_maze);

    findEntrance(&test_maze);
    printf("Entrance x: %d y: %d\n", test_maze.entrance_x, test_maze.entrance_y);

    maze_exit = exploreMaze(&test_maze, 1, 0, 0);
    printf("Exit status: %d\n", maze_exit);
    printf("Check exit: %d\n", isExit(&test_maze, 1, 0));
    printf("Check exit: %d\n", isExit(&test_maze, 1, 5));

    printMaze(&test_maze);

    readMazeSize(&test_maze, "(15 ,20)");
    printf("sizex: %d sizey: %d\n", test_maze.size_x, test_maze.size_y);

    printf("Route length %d\n", test_maze.route_length);

    for (i = 0; i < grid_size; i++) {
        for (j = 0; j < grid_size; j++) {
            printf("%c", test_maze.final_grid[i][j]);
        }
        printf("\n");
    }

    loadMaze("./Text Files/maze.txt", &test_maze);

    printMaze(&test_maze);
    freeMazeArray(&test_maze);
}
