#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>

#include "symbols.h"

/* Max number of tokens  in an INSTRUCT */
#define MAX_TOKENS 6

typedef enum type_t { FILE_REF,
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
    token_t* instr[MAX_TOKENS];
    token_t* token;
    char* filename;
    int pos;
    int len;
    int size;
} prog_t;

/* Mainly defined for a shorter version of unsigned int. However, also allows
 * buffer MAX to be changed easily. line_t must be unsigned for MAX macro to 
 * work
 */
typedef unsigned int line_t;

/* ------- TOKEN RETURN FUNCTIONS ------- */
/* Returns pointer to current token and increments position in program queue */
token_t* dequeueToken(prog_t* program);

/* Returns pointer to current token without incrementing program position */
token_t* peekToken(prog_t* program, int dist);

/* ------- TOKENIZATION FUNCTIONS ------- */
/* Parses file line by line, identifying tokens in each line read in. The
 * identified tokens are then added to the program queue ready for parsing */
prog_t* tokenizeFile(char* filename);

/* Handles adding token to stream. Resizes token array if necesarry and then
 * calls build token to create token entry in table
 */
void enqueueToken(prog_t* program, char* attrib, int len, int line, int word);

/* Is passed pointer to token_t* point in stream to build token. Attrib points
 * to beginning of "attrib" but is not null terminated. "len" allows the string
 * to be properly copied, while line and word simply reference the location of 
 * the token. Other parts of the token (such as type) are built within the 
 * function.
 */
void buildToken(token_t* token, char* attrib, int len, int line, int word);

/* Converts STRCON string i.e. "" or ## into normal string without quotes or 
 * hashes. If hashes are identified, rot18 is called to convert the string
 */
void getSTRCON(char* word);

/* Decodes ROT18 encoding */
void rot18(char* s);





/* ------ TOKEN STREAM INITIALISATION FUNCTIONS ------ */
/* Allocates memory for a prog_t struct to hold token stream 
 */
prog_t* initProgQueue(char* filename);

/* Resizes array for token stream if when full 
 */
void expandProgQueue(prog_t* program);

/* Frees all alloced memory for token stream 
 */
void freeProgQueue(prog_t* program);


/* ------ FILE & LINE HANDLING FUNCTIONS ------ */
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

/* Updates position pointer to point to beginning of token string, and returns
 * length of the string. This allows tokens to be identified even without NULL
 * termination. 
 */ 
line_t parseBufferWords(char** pos);

/* ------ TOKEN IDENTIFICATION FUNCTIONS ------- */
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

#endif
