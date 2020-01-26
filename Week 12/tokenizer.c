#include "tokenizer.h"
#include "error.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* Initilisation values for line buffer */
#define LINE_SIZE 256
#define FACTOR 2

/* Macro defines maximum value of line_t type */
#define LINE_T_MAX (line_t) ~0

/* Definitions of terminators for identifying individual tokens */
#define WHITESPACE " \t\n\v\f\r"
#define TERMINATORS "(){},=#\" \t\n\v\f\r\0"
#define STRING_TERMINATORS "#\"\n\0"
#define QUOTE_LEN 2

/* Initialisation value for program size */
#define PROG_LENGTH 500

/* Constants for ROT18 Encoding */
#define ROT5 5
#define ROT13 13
#define ALPHA 26
#define DIGIT 10

/* ------- TOKEN RETURN FUNCTIONS ------ */
token_t* dequeueToken(prog_t* program) {
    token_t* token;

    if (program->pos >= program->len) {
        dequeue_error(program);
    }

    token = program->token + program->pos;
    program->pos++;

    return token;
}

token_t* peekToken(prog_t* program, int dist) {
    if (program->pos + dist >= program->len) {
        dequeue_error(program);
    }

    return program->token + (program->pos + dist);
}

/* ------- TOKENIZATION FUNCTIONS ------ */
prog_t* tokenizeFile(char* filename) {
    FILE* file;
    prog_t* program;

    char* buffer = NULL;
    char* line_pos;
    line_t size;
    line_t line_len, word_len;
    int line_num = 0, word_num;

    if (!(file = fopen(filename, "r"))) {
        return NULL;
    }
    program = initProgQueue(filename);

    /* Return lines in the file */
    while ((line_len = getLine(&buffer, &size, file))) {
       /* truncateLineEnd(buffer, &line_len);*/
        line_pos = buffer;
        word_num = 0;

        /* Parse through words in the line */
        while ((word_len = parseBufferWords(&line_pos))) {
            enqueueToken(program, line_pos, word_len, line_num, word_num);
            line_pos += word_len;
            word_num++;
        }
        line_num++;
    }

    free(buffer);
    fclose(file);
    return program;
}

void enqueueToken(prog_t* program, char* attrib, int len, int line, int word) {
    int i = program->len;

    if (i >= program->size) {
        expandProgQueue(program);
    }

    buildToken(program->token + i, attrib, len, line, word);
    program->len++;
}

void buildToken(token_t* token, char* attrib, int len, int line, int word) {
    /* Allocate space for attrib and copy. Memcpy is used as string is not NULL
    terminated */
    char* str = (char*)malloc(sizeof(char) * (len + 1));
    if (!str) {
        ON_ERROR("Error allocating space for token attribute\n");
    }
    memcpy(str, attrib, sizeof(char) * len);
    str[len] = '\0';

    token->type = tokenType(str);
    token->line = line;
    token->word = word;

    /* Converts ROT18 encoded strings. COnverts strings as default */
    /* FIXME this is a bit dirty */
    if (token->type == STRCON) {
        /* getSTRCON converts string in place */
        getSTRCON(str);
    }

    token->attrib = str;
}

/* decode strconsts in place */
/* FIXME could covert escape characters here */
/* FIXME does't need first if statemtn */
void getSTRCON(char* word) {
    int len;

    len = strlen(word);
    /* Truncate " or # */
    word[len - 1] = '\0';

    if (word[0] == '#') {
        rot18(word + 1);
    }

    /* Memmove is compatible with in place shifting */
    memmove(word, word + 1, strlen(word + 1) + 1);
}

/* FIXME rename tok */
void rot18(char* s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (isupper(s[i])) {
            s[i] = 'A' + (s[i] - 'A' + ROT13) % ALPHA;
        } else if (islower(s[i])) {
            s[i] = 'a' + (s[i] - 'a' + ROT13) % ALPHA;
        } else if (isdigit(s[i])) {
            s[i] = '0' + (s[i] - '0' + ROT5) % DIGIT;
        }
    }
}

/* ------ TOKEN STREAM INITIALISATION FUNCTIONS ------ */
/* FIXME maybe clean up mallocs */
prog_t* initProgQueue(char* filename) {
    prog_t* tmp = (prog_t*)malloc(sizeof(prog_t));
    if (!tmp) {
        ON_ERROR("Error allocating program struct\n");
    }

    tmp->filename = (char*)malloc(sizeof(char) * (strlen(filename) + 1));
    if (!tmp->filename) {
        ON_ERROR("Error allocating memory for filename\n");
    }
    strcpy(tmp->filename, filename);

    tmp->token = (token_t*)malloc(sizeof(token_t) * PROG_LENGTH);
    if (!tmp->token) {
        ON_ERROR("Error allocating memory for program queue\n");
    }

    tmp->size = PROG_LENGTH;
    tmp->pos = 0;
    tmp->len = 0;

    return tmp;
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
    free(program->filename);
    free(program->token);
    free(program);
}

/* ------ FILE & LINE HANDLING FUNCTIONS ------ */
line_t getLine(char** buffer, line_t* size, FILE* file) {
    line_t i = 0;
    long int file_pos = ftell(file);

    /* If buffer has not been initialised, buffer is allocated and size is set */
    if (!*buffer) {
        *size = LINE_SIZE;
        *buffer = bufferAllocHandler(NULL, *size);
    }

    /* Use fgets to read into buffer. Space remaining in buffer is decremented 
       based on how far into the file has been read. Allows for dyanmic resizing
       of buffer. The while loop handles \n characters. */
    while (fgets(*buffer + i, *size - i, file)) {
        /* Get number of characters read */
        i = ftell(file) - file_pos;

        /* When '\n' read of the first time stream reaches the
        EOF, fgets will end up here. Can return number of characters read */
        if (feof(file) || (*buffer)[i - 1] == '\n') {
            return i;
        }

        /* If fgets stopped reading and last char wasn't \n of eof, buffer was 
        filled and needs to be expanded. Conditions below expand buffer up to
        maximum size of LINE_T_MAX, at which point the function will fail */
        if (*size == LINE_T_MAX) {
            ON_ERROR("Line buffer length exceeded. Exiting...\n");
        }

        if (LINE_T_MAX / FACTOR <= *size) {
            *size = LINE_T_MAX;
        } else {
            *size *= FACTOR;
        }
        *buffer = bufferAllocHandler(*buffer, *size);
    }
    /* Returns 0 on eof or error. This should be the only case where 0 can be
    returned as newlines will always include at least the line ending chars */
    return 0;
}

/* FIXME THis might not actually be needed */
void truncateLineEnd(char* buffer, line_t* len) {
    line_t size = *len;
    /* FIXME magic numbers */
    if (size && buffer[size - 1] == '\n') {
        if (size >= 2 && buffer[size - 2] == '\r') {
            buffer[size - 2] = '\0';
            *len = size - 2;
        } else {
            buffer[size - 1] = '\0';
            *len = size - 1;
        }
    }
}

char* bufferAllocHandler(char* buffer, line_t size) {
    char* tmp = (char*)realloc(buffer, sizeof(char) * size);
    if (!tmp) {
        if (!buffer) {
            ON_ERROR("Line buffer allocation failed\n");
        } else {
            ON_ERROR("Line buffer reallocation failed\n");
        }
    }
    return tmp;
}

line_t parseBufferWords(char** pos) {
    if (*pos[0] == '\0') {
        return 0;
    }
    /* Remove leading whitespace */
    *pos += strspn(*pos, WHITESPACE);

    /* Return len 1 on single character tokens. For " or #, need to offset by 1
    so that it doesn't stop on first char. It will return len up to " or #, so +2
    len is required. Other tokens can simply be read up to terminator */
    switch (*pos[0]) {
        case '(':
        case ')':
        case '{':
        case '}':
        case '=':
        case ',':
            return 1;
            break;
        case '"':
        case '#':
            return strcspn(*pos + 1, STRING_TERMINATORS) + QUOTE_LEN;
            break;
        default:
            return strcspn(*pos, TERMINATORS);
            break;
    }
}

/* ------- TOKEN IDENTIFICATION FUNCTIONS ------ */
type_t tokenType(char* word) {
    if (isFILE(word)) {
        return FILE_REF;
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
    if (isCOMMA(word)) {
        return COMMA;
    }
    return ERROR;
}

bool_t isFILE(char* word) {
    return strcmp(word, "FILE") ? FALSE : TRUE;
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

bool_t isSTRCON(char* word) {
    int len = strlen(word);
    if (((word[0] == '"') && (word[len - 1] == '"')) ||
        ((word[0] == '#') && (word[len - 1] == '#'))) {
        return TRUE;
    }
    return FALSE;
}

bool_t isNUMCON(char* word) {
    int i;
    bool_t dot = FALSE;

    /* Checks that there is only one decimal point in string */
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

bool_t isStrUpper(char* word) {
    int i;
    for (i = 1; word[i] != '\0'; i++) {
        if (!isupper(word[i])) {
            return FALSE;
        }
    }
    return TRUE;
}
