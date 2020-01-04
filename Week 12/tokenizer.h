#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>

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
                      ERROR
} type_t;

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

/* Mainly defined for a shorter version of unsigned int. However, also allows
 * buffer MAX to be changed easily. line_t must be unsigned for MAX macro to 
 * work
 */
typedef unsigned int line_t;

prog_t* initProgQueue(void);
token_t* dequeueToken(prog_t* program);
token_t* peekToken(prog_t* program, int dist);
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

void printInstr(type_t instr);

prog_t* tokenizeFile(char* filename);

line_t parseBufferWords(char** pos);

/* ------ FILE HANDLING FUNCTIONS ------ */
FILE* openFile(char* filename);

/* Reads line from a file. Function returns number of characters in the string
 * including \n or 0 on file end or error. Must be passed initialised buffer or
 * NULL value ptr
 */
line_t getLine(char** buffer, line_t* size, FILE* file);

/* Truncates line endings from lines. Can handle LF or CRLF. Uses short-circuit
 * evaluation to avoid reading outside array indices by checking for zero-length
 * strings and ending type. Does nothing to string if line ends aren't
 * found.
 */
void truncateLineEnd(char* buffer, line_t* len);

/* Handles malloc and reallocing buffer. On the initial call buffer should be 
 * passed as NULL. For resizing, the existing pointer should be passed.
 */
char* bufferAllocHandler(char* buffer, line_t size);

#endif
