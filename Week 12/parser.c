#include "parser.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROG_LENGTH 500
#define FACTOR 2

/* FIXME move these decalarations to general error file */
#define ON_ERROR(STR)     \
    fprintf(stderr, STR); \
    exit(EXIT_FAILURE)

void initProgQueue(prog_t* program) {
    program->token = (token_t*)malloc(sizeof(token_t) * PROG_LENGTH);
    if (!program->token) {
        ON_ERROR("Error allocating memory for program queue\n");
    }

    program->size = PROG_LENGTH;
    program->pos = 0;
}

void enqueueToken(prog_t* program, char* attrib, int len, int line, int word) {
    int i = program->pos;
    if (i >= program->size) {
        expandProgQueue(program);
    }

    buildToken(program->token + i, attrib, len, line, word);
    program->pos++;
    program->len++;
}

void buildToken(token_t* token, char* attrib, int len, int line, int word) {
    char* str = (char*)malloc(sizeof(char) * (len + 1));
    if (!str) {
        ON_ERROR("Error allocating space of token attribute\n");
    }
    memcpy(str, attrib, sizeof(char) * len);
    str[len] = '\0';

    token->attrib = str;
    token->type = tokenType(token->attrib);
    token->line = line;
    token->word = word;
}

void expandProgQueue(prog_t* program) {
    program->size *= FACTOR;
    program->token = (token_t*)realloc(program->token, program->size);
    if (!program->token) {
        ON_ERROR("Error resizing program queue\n");
    }
}

void freeProgQueue(prog_t* program) {
    int i;
    for (i = 0; i < program->len; i++) {
        free(program->token[i].attrib);
    }
    free(program->token);
}

type_t tokenType(char* word) {
    if (isFILE(word)) {
        return FILE_;
    }
    if (isABORT(word)) {
        return ABORT;
    }
    if (isIN2STR(word)) {
        return IN2STR;
    }
    if (isINNUM(word)) {
        return INNUM;
    }
    if (isIFEQUAL(word)) {
        return IFEQUAL;
    }
    if (isGREATER(word)) {
        return IFGREATER;
    }
    if (isINC(word)) {
        return INC;
    }
    if (isSET(word)) {
        return SET;
    }
    if (isJUMP(word)) {
        return JUMP;
    }
    if (isPRINT(word)) {
        return PRINT;
    }
    if (isPRINTN(word)) {
        return PRINTN;
    }
    if (isRND(word)) {
        return RND;
    }
    if (isSTRVAR(word)) {
        return STRVAR;
    }
    if (isNUMVAR(word)) {
        return NUMVAR;
    }
    if (isSTRCON(word)) {
        return STRCON;
    }
    if (isNUMCON(word)) {
        return NUMCON;
    }
    if (isBRACKET(word)) {
        return BRACKET;
    }
    if (isSECTION(word)) {
        return SECTION;
    }
    return ERROR;
}

bool_t isFILE(char* word) {
    return strcmp(word, "ABORT") ? FALSE : TRUE;
}

bool_t isABORT(char* word) {
    return strcmp(word, "ABORT") ? FALSE : TRUE;
}

bool_t isIN2STR(char* word) {
    return strcmp(word, "IN2STR") ? FALSE : TRUE;
}

bool_t isINNUM(char* word) {
    return strcmp(word, "INNUM") ? FALSE : TRUE;
}

bool_t isIFEQUAL(char* word) {
    return strcmp(word, "IFEQUAL") ? FALSE : TRUE;
}

bool_t isGREATER(char* word) {
    return strcmp(word, "IFGREATER") ? FALSE : TRUE;
}

bool_t isINC(char* word) {
    return strcmp(word, "INC") ? FALSE : TRUE;
}

bool_t isSET(char* word) {
    return strcmp(word, "=") ? FALSE : TRUE;
}

bool_t isJUMP(char* word) {
    return strcmp(word, "JUMP") ? FALSE : TRUE;
}

bool_t isPRINT(char* word) {
    return strcmp(word, "PRINT") ? FALSE : TRUE;
}

bool_t isPRINTN(char* word) {
    return strcmp(word, "PRINTN") ? FALSE : TRUE;
}

bool_t isRND(char* word) {
    return strcmp(word, "RND") ? FALSE : TRUE;
}

bool_t isSTRVAR(char* word) {
    if (word[0] == '$') {
        if (isStrUpper(word)) {
            return TRUE;
        }
    }
    return FALSE;
}

bool_t isNUMVAR(char* word) {
    if (word[0] == '%') {
        if (isStrUpper(word)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* FIXME Error messages for strings? */
bool_t isSTRCON(char* word) {
    int len = strlen(word);
    if (((word[0] == '"') && (word[len - 1] == '"')) ||
        ((word[0] == '#') && (word[len - 1] == '#'))) {
        return TRUE;
    }
    return FALSE;
}

/* QUESTION Does the first digit have to be a number? */
bool_t isNUMCON(char* word) {
    int i;
    bool_t dot = FALSE;

    for (i = 0; word[i] != '\0'; i++) {
        if (!isdigit(word[i])) {
            if (word[i] == '.' && dot == FALSE) {
                dot = TRUE;
            } else {
                return FALSE;
            }
        }
    }
    return TRUE;
}

bool_t isBRACKET(char* word) {
    if (!strcmp(word, "(") || !strcmp(word, ")")) {
        return TRUE;
    }
    return FALSE;
}

bool_t isSECTION(char* word) {
    if (!strcmp(word, "{") || !strcmp(word, "}")) {
        return TRUE;
    }
    return FALSE;
}

bool_t isCOMMA(char* word) {
    return strcmp(word, ",") ? FALSE : TRUE;
}

/* FIXME Error handling of invalid variable name */
bool_t isStrUpper(char* word) {
    int i;
    for (i = 1; word[i] != '\0'; i++) {
        if (!isupper(word[i])) {
            return FALSE;
        }
    }
    return TRUE;
}
