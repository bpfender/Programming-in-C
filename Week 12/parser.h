#ifndef PARSER_H
#define PARSER_H

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
                      COMMA,
                      ERROR } type_t;

typedef enum bool_t { FALSE,
                      TRUE } bool_t;

typedef struct token_t {
    type_t type;
    char* attrib;
    int line;
    int word;
} token_t;

typedef struct prog_t {
    token_t* token;
    int pos;
    int len;
    int size;

} prog_t;

void initProgQueue(prog_t* program);
void enqueueToken(prog_t* program, char* attrib, int len, int line, int word);
void buildToken(token_t* token, char* attrib, int len, int line, int word);
void expandProgQueue(prog_t* program);
void freeProgQueue(prog_t* program);

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
bool_t isCOMMA(char* word);
bool_t isStrUpper(char* word);

/*    switch (token->type) {
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
        case COMMA:
        case ERROR:
        default:
            break;
    }
*/

#endif
