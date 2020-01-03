#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIMPLE_BRACKET 3

/* Can this be defined with pos++ too */
#define CURR_TOKEN(program) \
    &(program->token[program->pos]);

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
    token_t* token = CURR_TOKEN(program);
    program->pos++;

    switch (token->type) {
        case FILE_:
            file(program);
            break;
        case ABORT:
            abort(program);
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

    token_t* token = CURR_TOKEN(program);
    program->pos++;

    if (token->type == STRCON) {
        getSTRCON(token->attrib);
        next_program = tokenizeFile(token->attrib);
        prog(next_program);
        instr(program);
    } else {
        handleError();
    }
}

void abort(prog_t* program) {
    token_t* token = CURR_TOKEN(program);
    program->pos++;

    if (token->type == BRACKET && !strcmp(token->attrib, "}")) {
        instr(program);
    } else {
        handleError();
    }
}

void in2str(prog_t* program) {
    token_t* token = CURR_TOKEN(program);
    program->pos++;
}

void innum(prog_t* program) {}
void ifequal(prog_t* program) {}
void ifgreater(prog_t* program) {}

void inc(prog_t* program) {
    token_t* token = CURR_TOKEN(program);
    program->pos++;
}

void set(prog_t* program) {}

void jump(prog_t* program) {
    token_t* token = CURR_TOKEN(program);
    program->pos++;

    if (token->type == NUMCON) {
        instr(program);
    } else {
        handleError();
    }
}

void print(prog_t* program) {
    token_t* token = CURR_TOKEN(program);
    program->pos++;

    if (token->type == STRVAR || token->type == NUMVAR ||
        token->type == STRCON || token->type == NUMCON) {
        instr(program);
    } else {
        handleError();
    }
}

void rnd(prog_t* program) {
    token_t* token = CURR_TOKEN(program);
    program->pos++;
}
void var(prog_t* program) {}

void handleError(void) {
    ON_ERROR("PANIC ERROR\n");
}

bool_t checkSingleBracket(prog_t* program, type_t* instr) {
    int i;
    token_t* token[SIMPLE_BRACKET];
    for (i = 0; i < SIMPLE_BRACKET; i++) {
        token[i] = CURR_TOKEN(program);
        program->pos++;
    }

    if (!strcmp(token[0]->attrib, "(") &&
        token[1]->type == instr &&
        !strcmp(token[2]->attrib, ")")) {
        return TRUE;
    }
    return FALSE;
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