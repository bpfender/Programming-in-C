#include "interpreter.h"
#include "parser.h"

int main(int argc, char* argv[]) {
    prog_t* program;
    symbol_t* symbols;

    if (argc != 2) {
        fprintf(stderr, "Expected file as argument to function\n");
        exit(EXIT_FAILURE);
    }

    if (!(program = tokenizeFile(argv[1]))) {
        fprintf(stderr, "Error opening %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

#ifdef INTERP
    inter_rndSeed();
#endif

    symbols = initSymbolTable();
    addFilename(symbols, argv[1], NULL);

    if (parseFile(program, symbols)) {
        printf("Parsed OK\n");
    } else {
        printf("Parse Error\n");
    }

    freeProgQueue(program);
    freeSymbolTable(symbols);

    return 0;
}
