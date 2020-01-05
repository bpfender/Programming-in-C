#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME need a gettoken function for bounds checking */
/* FIXME functionise bracket handling */

#define RND_ARGS 1
#define IFCOND_ARGS 2
#define IN2STR_ARGS 2
#define INNUM_ARGS 1
#define INC_ARGS 1

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

#define ERROR(TOKEN)                                                                                           \
    {                                                                                                          \
        fprintf(stderr, "Error code line %d\n Program line %d word %d\n", __LINE__, TOKEN->line, TOKEN->word); \
        exit(2);                                                                                               \
    }

void parseFile(char* filename) {
    prog_t* program = tokenizeFile(filename);

    prog(program);
    printf("Parsed ok\n");
    freeProgQueue(program);
}

void prog(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (!strcmp(token->attrib, "{")) {
        instr(program);
    } else {
        ERROR(token);
    }
}

void instr(prog_t* program) {
    token_t* token = dequeueToken(program);

    switch (token->type) {
        case FILE_:
            file(program);
            break;
        case ABORT:
            prog_abort(program);
            break;
        case IN2STR:
            in2str(program);
            break;
        case INNUM:
            innum(program);
            break;
        case IFEQUAL:
            ifequal(program);
            break;
        case IFGREATER:
            ifgreater(program);
            break;
        case INC:
            inc(program);
            break;
        case SET:
            ERROR(token);
            break;
        case JUMP:
            jump(program);
            break;
        case PRINT:
            print(program);
            break;
        case PRINTN:
            print(program);
            break;
        case RND:
            rnd(program);
            break;
        case STRVAR:
        case NUMVAR:
            var(program, token->type);
            break;
        case SECTION:
            if (!strcmp(token->attrib, "}")) {
                break;
            }
            ERROR(token);
            break;
        case STRCON:
        case NUMCON:
        case BRACKET:
        case COMMA:
        case ERROR:
        default:
            ERROR(token);
            break;
    }
}

/* FIXME super dirty filenmae hack */
void file(prog_t* program) {
    char filename[500] = "./Files/";
    prog_t* next_program;

    token_t* token = dequeueToken(program);

    if (token->type == STRCON) {
        getSTRCON(token->attrib);
        strcat(filename, token->attrib);
        next_program = tokenizeFile(filename);

        prog(next_program);
        freeProgQueue(next_program);

        instr(program);
    } else {
        ERROR(token);
    }
}

void prog_abort(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (token->type == SECTION && !strcmp(token->attrib, "}")) {
        instr(program);
    } else {
        ERROR(token);
    }
}

void in2str(prog_t* program) {
    if (parseBrackets(program, STRVAR, IN2STR_ARGS)) {
        instr(program);

    } else {
        ERROR(peekToken(program, 0));
    }
}

void innum(prog_t* program) {
    if (parseBrackets(program, NUMVAR, INNUM_ARGS)) {
        instr(program);

    } else {
        ERROR(peekToken(program, 0));
    }
}

/* FIXME ifequal and ifgreater are identical */
/* QUESTION Does this need two sets of instr for {} */
void ifequal(prog_t* program) {
    token_t* token = peekToken(program, 0);
    if (parseCondBracket(program)) {
        token = dequeueToken(program);

        /* Could just reuse prog(); */
        if (!strcmp(token->attrib, "{")) {
            instr(program);
            return;
        } else {
            ERROR(token);
            return;
        }

    } else {
        ERROR(token);
        return;
    }
}

void ifgreater(prog_t* program) {
    token_t* token = peekToken(program, 0);
    if (parseCondBracket(program)) {
        token = dequeueToken(program);

        if (!strcmp(token->attrib, "{")) {
            instr(program);
            return;
        } else {
            ERROR(token);
            return;
        }

    } else {
        ERROR(token);
    }
}

void inc(prog_t* program) {
    if (parseBrackets(program, NUMVAR, INC_ARGS)) {
        instr(program);

    } else {
        ERROR(peekToken(program, 0));
    }
}

void jump(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (token->type == NUMCON) {
        instr(program);
    } else {
        ERROR(token);
    }
}

void print(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (token->type == STRVAR || token->type == NUMVAR ||
        token->type == STRCON || token->type == NUMCON) {
        instr(program);
    } else {
        ERROR(token);
    }
}

void rnd(prog_t* program) {
    if (parseBrackets(program, NUMVAR, RND_ARGS)) {
        instr(program);

    } else {
        ERROR(peekToken(program, 0));
    }
}

void var(prog_t* program, type_t var) {
    token_t* token = dequeueToken(program);

    switch (var) {
        case STRVAR:
            if (token->type == SET) {
                token = dequeueToken(program);
                if (token->type == STRCON || token->type == STRVAR) {
                    instr(program);
                    return;
                }
            }
            ERROR(token);
            break;
        case NUMVAR:
            if (token->type == SET) {
                token = dequeueToken(program);
                if (token->type == NUMVAR || token->type == NUMCON) {
                    instr(program);
                    return;
                }
            }
            ERROR(token);
            break;
        default:
            ERROR(token);
            break;
    }
}

bool_t parseSingleBracket(prog_t* program, type_t arg) {
    token_t* token = dequeueToken(program);
    if (strcmp(token->attrib, "(")) {
        return FALSE;
    }
    token = dequeueToken(program);
    if (token->type != arg) {
        return FALSE;
    }
    token = dequeueToken(program);
    if (strcmp(token->attrib, ")")) {
        return FALSE;
    }
    return TRUE;
}

bool_t parseCondBracket(prog_t* program) {
    token_t* token = dequeueToken(program);
    type_t type;

    if (strcmp(token->attrib, "(")) {
        return FALSE;
    }
    token = dequeueToken(program);
    if (!(token->type == STRVAR || token->type == NUMVAR ||
          token->type == STRCON || token->type == NUMCON)) {
        return FALSE;
    }
    type = token->type;

    token = dequeueToken(program);
    if (token->type != COMMA) {
        return FALSE;
    }

    token = dequeueToken(program);
    switch (type) {
        case STRVAR:
        case STRCON:
            if (!(token->type == STRVAR || token->type == STRCON)) {
                return FALSE;
            }
            break;
        case NUMVAR:
        case NUMCON:
            if (!(token->type == NUMVAR || token->type == NUMCON)) {
                return FALSE;
            }
            break;
        default:
            break;
    }

    token = dequeueToken(program);
    if (strcmp(token->attrib, ")")) {
        return FALSE;
    }

    return TRUE;
}

/* FIXME this is horribly written at the moment */
bool_t parseBrackets(prog_t* program, type_t arg, int n) {
    int i;
    token_t* token = dequeueToken(program);
    if (strcmp(token->attrib, "(")) {
        return FALSE;
    }

    for (i = 0; i < n - 1; i++) {
        token = dequeueToken(program);
        if (token->type != arg) {
            return FALSE;
        }
        token = dequeueToken(program);
        if (token->type != COMMA) {
            return FALSE;
        }
    }
    token = dequeueToken(program);
    if (token->type != arg) {
        return FALSE;
    }
    token = dequeueToken(program);
    if (strcmp(token->attrib, ")")) {
        return FALSE;
    }
    return TRUE;
}

void handleError(void) {
    ON_ERROR("PANIC ERROR\n");
}

/* decode strconsts in place */
void getSTRCON(char* word) {
    int len;
    if (word[0] == '"' || word[0] == '#') {
        len = strlen(word);
        word[len - 1] = '\0';

        if (word[0] == '#') {
            rot18(word + 1);
        }

        memmove(word, word + 1, sizeof(char) * len - 1);
    }
}

void rot18(char* s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (isupper(s[i])) {
            s[i] = 'A' + (s[i] - 'A' + ROT13) % ALPHA;
        } else if (islower(s[i])) {
            s[i] = 'a' + (s[i] - 'a' + ROT13) % ALPHA;
        } else if (isdigit(s[i])) {
            s[i] = (s[i] + ROT5) % DIGIT;
        }
    }
}
