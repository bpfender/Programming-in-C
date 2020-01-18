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
