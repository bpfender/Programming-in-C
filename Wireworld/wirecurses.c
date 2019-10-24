#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neillncurses.h"

#define GRID_SIZE 40
#define ROWS GRID_SIZE
#define COLS GRID_SIZE

/* Exit codes for diagnosis of states when reading and checking file */
#define OPEN_ERR -1
#define SYMB_ERR -2
#define SIZE_ERR -3
#define READ_ERR -4
#define LINE_ERR -5
#define SUCCESS 0
#define LINE_READ 1

/* Contains current and next generation of circuit, as well as coordinates.
 * 2D arrays (grid_a & grid_b) should not be accessed directly. Instead use 
 * "current" and "next" pointers. Pointers are defined so that "copying"
 * next array into current array just involves swapping pointers.
 */
typedef struct circuit_struct {
    char (*current)[COLS];
    char (*next)[COLS];
    int x;
    int y;
    char grid_a[ROWS][COLS];
    char grid_b[ROWS][COLS];
} circuit_struct;

/* ------ SIMULATION FUNCTIONS ------- */
void nextGeneration(circuit_struct* circuit);
void updateCurrent(circuit_struct* circuit);
void nextCellState(circuit_struct* circuit);
int checkMooreGrid(circuit_struct* circuit);
int checkBounds(int x, int y);

/* ------ UTILITY & FILE READING FUNCTIONS ------ */
void initStruct(circuit_struct* circuit);
int loadCircuitFile(char* filename, circuit_struct* circuit);
int checkFileSize(FILE* file);
int readCircuit(circuit_struct* circuit, FILE* file);
int readLine(char line[COLS], FILE* fp);
int checkCircuit(char circuit[ROWS][COLS]);
int checkSymbol(char c);
void test(void);

int main(int argc, char* argv[]) {
    NCURS_Simplewin sw;

    circuit_struct circuit;
    initStruct(&circuit);

    /*test();*/

    if (argc != 2) {
        fprintf(stderr,
                "ERROR: Incorrect usage, try e.g. %s wirewcircuit1.txt\n",
                argv[0]);
        return 1;
    }
    if (loadCircuitFile(argv[1], &circuit) != SUCCESS) {
        fprintf(stderr, "Exiting...\n");
        return 1;
    }

    /* Initialise NCURS and display circuit world */
    Neill_NCURS_Init(&sw);
    Neill_NCURS_CharStyle(&sw, "H", COLOR_BLUE, COLOR_BLUE, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, "t", COLOR_RED, COLOR_RED, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, "c", COLOR_YELLOW, COLOR_YELLOW, A_NORMAL);
    Neill_NCURS_CharStyle(&sw, " ", COLOR_BLACK, COLOR_BLACK, A_NORMAL);

    do {
        Neill_NCURS_PrintArray(&circuit.current[0][0], COLS, ROWS, &sw);
        Neill_NCURS_Delay(200.0);
        Neill_NCURS_Events(&sw);
        nextGeneration(&circuit);
    } while (!sw.finished);
    Neill_NCURS_Done();

    return 0;
}

/* ------ SIMULATION FUNCTIONS ------ */

/* Iterate through circuit cells. x and y are being modified in circuit_struct
 * directly so that the correct cell is checked when nextCellState() is called
 */
void nextGeneration(circuit_struct* circuit) {
    int* x = &circuit->x;
    int* y = &circuit->y;

    for (*x = 0; *x < COLS; (*x)++) {
        for (*y = 0; *y < ROWS; (*y)++) {
            nextCellState(circuit);
        }
    }
    /* Newly calculated circuit grid becomes current circuit. */
    updateCurrent(circuit);
}

/* Swaps pointers of current and next, so that the "current" pointer points to 
 * the newly calculated next array and "next" points to the old array that can 
 * be overwritten 
 */
void updateCurrent(circuit_struct* circuit) {
    char(*tmp)[COLS];

    tmp = circuit->current;
    circuit->current = circuit->next;
    circuit->next = tmp;
}

/* Checks what the next cell state should be based on current x/y-coordinates 
 * in the circuit_struct and returns corresponding value. Modifies array 
 * pointed to by next directly to avoid writing ' ' repeatedly
 */
void nextCellState(circuit_struct* circuit) {
    int neigbors;
    char current = circuit->current[circuit->y][circuit->x];
    char* next = &circuit->next[circuit->y][circuit->x];

    switch (current) {
        case 'H':
            *next = 't';
            break;
        case 't':
            *next = 'c';
            break;
        case 'c':
            neigbors = checkMooreGrid(circuit);
            if (neigbors == 1 || neigbors == 2) {
                *next = 'H';
                break;
            }
            *next = 'c';
            break;
    }
}

/* Checks surrounding 8 cells for heads, returning number of heads surrounding 
 * the cell. Takes whole circuit_struct as argument to allow additional bounds 
 * checking in checkBounds() based on x,y coordinates.
 */
int checkMooreGrid(circuit_struct* circuit) {
    int i, j;
    int head_count = 0;
    int x = circuit->x;
    int y = circuit->y;

    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            /* Check that coordinate is within circuit grid and ignore the 
             * central cell. Checking the central cell isn't necesarry here, 
             * but may improve portability of code
             */
            if (checkBounds(x + i, y + j) && !(i == 0 && j == 0)) {
                head_count += (circuit->current[y + j][x + i] == 'H');
            }
        }
    }

    return head_count;
}

/* Checks that the coordinates tested fall within the bounds of the circuit
 * grid. If they don't, returns 0;
 */
int checkBounds(int x, int y) {
    return (x >= 0 && x < COLS) &&
           (y >= 0 && y < ROWS);
}

/* ------ UTILITY AND FILE HANDLING FUNCTIONS ------ */

/* Point current and next pointer to 2D circuit grids */
void initStruct(circuit_struct* circuit) {
    circuit->current = circuit->grid_a;
    circuit->next = circuit->grid_b;
}

int loadCircuitFile(char* filename, circuit_struct* circuit) {
    FILE* file;
    int exit_code;

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open \"%s\"\n", filename);
        return OPEN_ERR;
    }

    if (checkFileSize(file) == SIZE_ERR) {
        fprintf(stderr, "Unexpected file size\n");
        fclose(file);
        return SIZE_ERR;
    }

    if ((exit_code = readCircuit(circuit, file)) != SUCCESS) {
        fclose(file);
        return exit_code;
    }

    if (checkCircuit(circuit->current) == SYMB_ERR) {
        fclose(file);
        return SYMB_ERR;
    }
    /* Copy circuit so that spaces are all popuplated in right place. Avoids 
       having to repeately write spaces when calling nextCellState() */
    memcpy(circuit->next, circuit->current, ROWS * COLS);

    fclose(file);
    return SUCCESS;
}

/* File should always be (40 + \n) * 40 characters long. This does a quick
 * sense check that the file is the expected size so that it can fit into
 * circuit array
 */
int checkFileSize(FILE* file) {
    fseek(file, 0, SEEK_END);
    if (ftell(file) != (COLS + 1) * ROWS) {
        return SIZE_ERR;
    }
    rewind(file);
    return SUCCESS;
}

/* Reads whole circuit file and passes through exit_codes generated within
 * readLine()
 */
int readCircuit(circuit_struct* circuit, FILE* file) {
    int line = 0;
    int exit_code;

    while ((exit_code =
                readLine(circuit->current[line], file)) == LINE_READ) {
        line++;
    }

    if (exit_code == READ_ERR) {
        fprintf(stderr, "Error reading file\n");
    } else if (exit_code == LINE_ERR) {
        fprintf(stderr, "Unexpected line length in line %d\n", line + 1);
    }

    return exit_code;
}

/* Reads lines based on expected length of CIRC_GRID + '\n' characters. If the
 * line doesn't follow this format, an error will be returned. Returns LINE_READ
 * if line is read successfully, or SUCCESS when EOF is reached. If line is
 * returns relevant error code.
 */
int readLine(char line[COLS], FILE* file) {
    int c, i;

    for (i = 0; i < COLS + 1; i++) {
        c = getc(file);

        if (c == EOF) {
            if (feof(file)) {
                return SUCCESS;
            } else {
                return READ_ERR;
            }
        }
        /* '\n' character is expected at end of 40 char line */
        if ((i < COLS && c == '\n') ||
            (i == COLS && c != '\n')) {
            return LINE_ERR;
        }
        /* Read in only circuit chars, not '\n' at end of line */
        if (i < COLS) {
            line[i] = (char)c;
        }
    }
    return LINE_READ;
}

/* Iterates through complete circuit array to check for any errors. Lists
 * locations of any invalid characters
 */
int checkCircuit(char circuit[ROWS][COLS]) {
    int i, j;
    int exit_code = SUCCESS;

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (checkSymbol(circuit[i][j]) == SYMB_ERR) {
                fprintf(stderr,
                        "Invalid symbol \"%c\" in line %d, column %d\n",
                        circuit[i][j], i + 1, j + 1);
                exit_code = SYMB_ERR;
            }
        }
    }

    return exit_code;
}

int checkSymbol(char c) {
    switch (c) {
        case 'H':
        case 't':
        case 'c':
        case ' ':
            return SUCCESS;
    }
    return SYMB_ERR;
}

void test(void) {
    int i, j;

    circuit_struct test_circuit;
    char test_grid[ROWS][COLS];
    FILE* test_file;

    /* Check that array pointers in struct are correctly assigned to point to
     * 2D array and that arrays get updated properly
     */
    initStruct(&test_circuit);
    assert(test_circuit.current == test_circuit.grid_a &&
           test_circuit.next == test_circuit.grid_b);

    test_circuit.grid_a[5][5] = 'c';
    test_circuit.grid_b[25][35] = 'H';
    assert(test_circuit.current[5][5] == 'c');
    assert(test_circuit.next[25][35] = 'H');

    /* Check that pointers to arrays are swapped correctly 
     */
    updateCurrent(&test_circuit);
    assert(test_circuit.current == test_circuit.grid_b &&
           test_circuit.next == test_circuit.grid_a);

    /* Check that symbols are accepted/rejected correctly
     */
    assert(checkSymbol('c') == SUCCESS);
    assert(checkSymbol('H') == SUCCESS);
    assert(checkSymbol('t') == SUCCESS);
    assert(checkSymbol(' ') == SUCCESS);
    assert(checkSymbol('o') == SYMB_ERR);
    assert(checkSymbol('\0') == SYMB_ERR);

    /* Check that bounds checking returns false if x/y coordinates will fall
     * outside of bounds of circuit array 
     */
    assert(checkBounds(0, 0) == 1);
    assert(checkBounds(25, 25) == 1);
    assert(checkBounds(5, 40) == 0);
    assert(checkBounds(-1, 25) == 0);
    assert(checkBounds(40, -1) == 0);

    /* Populate a 40x40 array randomly with valid characters and check that it
     * is recognised as valid
     */
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            switch (rand() % 4) {
                case (0):
                    test_grid[i][j] = 'c';
                    break;
                case (1):
                    test_grid[i][j] = 'H';
                    break;
                case (2):
                    test_grid[i][j] = 't';
                    break;
                case (3):
                    test_grid[i][j] = ' ';
                    break;
            }
        }
    }
    assert(checkCircuit(test_grid) == SUCCESS);

    /* Insert invalid character and check that it is recognised correctly
     */
    test_grid[25][36] = 'o';
    assert(checkCircuit(test_grid) == SYMB_ERR);

    /* Check line function returns as expected
     */
    test_file = fopen("./Test/testline.txt", "r");
    if (test_file == NULL) {
        fprintf(stderr, "Cannot open \"testline.txt\"\n");
        exit(EXIT_FAILURE);
    }
    /* First read of full line should return line successfully read */
    assert(readLine(test_circuit.current[0], test_file) == LINE_READ);
    /* Second read should return EOF file reached */
    assert(readLine(test_circuit.current[0], test_file) == SUCCESS);
    fclose(test_file);

    /* Check that read file funtion reads in whole circuit correctly */
    test_file = fopen("./Test/testcircuit.txt", "r");
    if (test_file == NULL) {
        fprintf(stderr, "Cannot open \"testcircuit.txt\"\n");
        exit(EXIT_FAILURE);
    }
    /* File should read in successfully */
    assert(readCircuit(&test_circuit, test_file) == SUCCESS);
    /* Check that last charcter in file is in last array element */
    assert(test_circuit.current[ROWS - 1][COLS - 1] == 'c');
    fclose(test_file);

    /* Check that various error flags are returned correctly from 
     * loadCircuitFile() function if invalid files are read
     */
    assert(loadCircuitFile("invalidname.txt", &test_circuit) == OPEN_ERR);
    assert(loadCircuitFile("./Test/invalidsymbol.txt", &test_circuit) == SYMB_ERR);
    assert(loadCircuitFile("./Test/invalidsize.txt", &test_circuit) == SIZE_ERR);
    assert(loadCircuitFile("./Test/invalidline.txt", &test_circuit) == LINE_ERR);
    assert(loadCircuitFile("./Test/testcircuit.txt", &test_circuit) == SUCCESS);

    /* Check individual Moore Neighborhoods to check they are returned correctly
     * and check that next cell state is returned correctly.
     */
    test_circuit.x = 0;
    test_circuit.y = 0;
    assert(checkMooreGrid(&test_circuit) == 2);
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == 'H');

    test_circuit.x = 6;
    test_circuit.y = 4;
    assert(checkMooreGrid(&test_circuit) == 1);
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == 'H');

    test_circuit.x = 11;
    test_circuit.y = 5;
    assert(checkMooreGrid(&test_circuit) == 3);
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == 'c');

    test_circuit.x = 2;
    test_circuit.y = 5;
    assert(checkMooreGrid(&test_circuit) == 0);
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == 'c');

    test_circuit.x = 39;
    test_circuit.y = 39;
    assert(checkMooreGrid(&test_circuit) == 3);
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == 'c');

    /* More tests of next cell states
     */
    test_circuit.x = 25;
    test_circuit.y = 25;
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == ' ');

    test_circuit.x = 7;
    test_circuit.y = 10;
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == 'c');

    test_circuit.x = 6;
    test_circuit.y = 10;
    nextCellState(&test_circuit);
    assert(test_circuit.next[test_circuit.y][test_circuit.x] == 't');

    /* Check that next generation correctly updates cells in "current" array
     */
    nextGeneration(&test_circuit);
    assert(test_circuit.current[4][6] == 'H');
    assert(test_circuit.current[10][5] == 'H');
    assert(test_circuit.current[0][0] == 'H');
    assert(test_circuit.current[4][5] == 't');
    assert(test_circuit.current[6][12] == 't');
    assert(test_circuit.current[5][10] == 'c');
    assert(test_circuit.current[8][24] == 'c');
    assert(test_circuit.current[39][39] == 'c');
    assert(test_circuit.current[25][25] == ' ');
    assert(test_circuit.current[4][11] == ' ');
}