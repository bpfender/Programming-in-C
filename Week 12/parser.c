#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_TOKEN_LEN 50

/*FIXME better alternative for INSTR_FILE */
typedef enum type_t { FILE_,
                      ABORT,
                      IN2STR,
                      INNUM,
                      IFEQUAL,
                      IFGREATER,
                      INC,
                      SET,
                      JUMP,
                      PRINT,
                      PRINTN,
                      RND,
                      STRVAR,
                      NUMVAR,
                      STRCON,
                      NUMCON,
                      BRACKET,
                      SECTION,
                      ERROR } type_t;

typedef enum bool_t { FALSE,
                      TRUE } bool_t;

typedef struct pos_t {
    int line;
    int col;
    long file_pos;
} pos_t;

typedef struct token_t {
    type_t type;
    char attrib[50];
    int line;
    int col;
} token_t;

void buildToken(token_t* token, char* word, int line, int col);

/* Token identification functions */
type_t tokenType(char* word);
bool_t isFILE(char* word);
bool_t isABORT(char* word);
bool_t isIN2STR(char* word);
bool_t isINNUM(char* word);
bool_t isIFEQUAL(char* word);
bool_t isGREATER(char* word);
bool_t isINC(char* word);
bool_t isSET(char* word);
bool_t isJUMP(char* word);
bool_t isPRINT(char* word);
bool_t isPRINTN(char* word);
bool_t isRND(char* word);
bool_t isSTRVAR(char* word);
bool_t isNUMVAR(char* word);
bool_t isSTRCON(char* word);
bool_t isNUMCON(char* word);
bool_t isBRACKET(char* word);
bool_t isSECTION(char* word);
bool_t isStrUpper(char* word);

void test(void);

int main(void) {
    test();
    return 0;
}

void buildToken(token_t* token, char* word, int line, int col) {
    token->type = tokenType(word);
    strcpy(token->attrib, word);
    token->line = line;
    token->col = col;

    switch (token->type) {
        case FILE_:
        case ABORT:
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
        case ERROR:
        default:
            break;
    }
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

void test(void) {
}