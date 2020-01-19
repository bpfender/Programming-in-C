#include "syntax.h"

#define ERROR(TOKEN)                                                                                           \
    {                                                                                                          \
        fprintf(stderr, "Error code line %d\n Program line %d word %d\n", __LINE__, TOKEN->line, TOKEN->word); \
        exit(2);                                                                                               \
    }

void parse_prog(token_t* token) {
    if (strcmp(token->attrib), "{") {
        ERROR(token);
    }
}

void parse_section(token_t* token) {
    if (strcmp(token->attrib), "}") {
        ERROR(token);
    }
}

bool_t parse_file(token_t* token, symbol_t* symbols) {
}

void parse_abort(token_t* token) {
    if (strcmp(token->attrib, "}")) {
        ERROR(token);
    }
}

void parse_set(prog_t* program){}
