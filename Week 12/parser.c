#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME need a gettoken function for bounds checking */

#define RND_ARGS 1
#define IFCOND_ARGS 2
#define IN2STR_ARGS 2
#define INNUM_ARGS 1
#define INC_ARGS 1

#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

void prog(prog_t* program) {
    token_t* token = program->token;

    if (token->type == BRACKET && !strcmp(token->attrib, "{")) {
        program->pos++;
        instr(program);
    } else {
        handleError();
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
        case IN2STR:
        case INNUM:
        case IFEQUAL:
        case IFGREATER:
        case INC:
        case SET:
        case JUMP:
        case PRINT:
        case PRINTN:
        case RND:
        case STRVAR:
        case NUMVAR:
        case STRCON:
        case NUMCON:
        case BRACKET:
        case SECTION:
        case COMMA:
        case ERROR:
        default:
            break;
    }
}

void file(prog_t* program) {
    prog_t* next_program;

    token_t* token = dequeueToken(program);

    if (token->type == STRCON) {
        getSTRCON(token->attrib);
        next_program = tokenizeFile(token->attrib);
        prog(next_program);
        instr(program);
    } else {
        handleError();
    }
}

void prog_abort(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (token->type == BRACKET && !strcmp(token->attrib, "}")) {
        instr(program);
    } else {
        handleError();
    }
}

void in2str(prog_t* program) {
    

}

void innum(prog_t* program) {
}

void ifequal(prog_t* program) {
    token_t* token = dequeueToken(program);
}

void ifgreater(prog_t* program) {
    
}

void inc(prog_t* program) {

}

void jump(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (token->type == NUMCON) {
        instr(program);
    } else {
        handleError();
    }
}

void print(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (token->type == STRVAR || token->type == NUMVAR ||
        token->type == STRCON || token->type == NUMCON) {
        instr(program);
    } else {
        handleError();
    }
}

void rnd(prog_t* program) {

}

void var(prog_t* program) {
    token_t* token = dequeueToken(program);

    if (token->type == SET) {
        token = dequeueToken(program);
        if (token->type == STRCON || token->type == STRVAR ||
            token->type == NUMCON || token->type == STRCON) {
            instr(program);
            return;
        }
    }
    handleError();
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
