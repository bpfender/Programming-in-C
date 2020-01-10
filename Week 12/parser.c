#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbols.h"

/* FIXME need a gettoken function for bounds checking */
/* FIXME functionise bracket handling */

#define TWO_ARG_LEN 5
#define ONE_ARG_LEN 3

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
    char* p;
    symbol_t* symbols = initSymbolTable();
    ast_t* ast = initAST();

    prog_t* program = tokenizeFile(filename, symbols);

    prog(program, symbols, ast);
    printf("Parsed ok\n");
    p = mvm_print(symbols->files);
    printf("%s\n", p);
    free(p);
    p = mvm_print(symbols->vars);
    printf("%s\n", p);
    free(p);
    freeProgQueue(program);
    freeSymbolTable(symbols);
}

void prog(prog_t* program, symbol_t* symbols, ast_t* ast) {
    token_t* token = dequeueToken(program);

    if (!strcmp(token->attrib, "{")) {
        instr(program, symbols, ast);
    } else {
        ERROR(token);
    }
}

void instr(prog_t* program, symbol_t* symbols, ast_t* ast) {
    token_t* token = dequeueToken(program);

    switch (token->type) {
        case FILE_:
            file(program, symbols, ast);
            break;
        case ABORT:
            prog_abort(program, symbols, ast);
            return;
            break;
        case IN2STR:
            in2str(program, symbols, ast);
            break;
        case INNUM:
            innum(program, symbols, ast);
            break;
        case IFEQUAL:
            ifequal(program, symbols, ast);
            break;
        case IFGREATER:
            ifgreater(program, symbols, ast);
            break;
        case INC:
            inc(program, symbols, ast);
            break;
        case SET:
            ERROR(token);
            break;
        case JUMP:
            jump(program, symbols, ast);
            break;
        case PRINT:
        case PRINTN:
            print(program, token->type, symbols, ast);
            break;
        case RND:
            rnd(program, symbols, ast);
            break;
        case STRVAR:
        case NUMVAR:
            set(program, token->type, symbols, ast);
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
void file(prog_t* program, symbol_t* symbols, ast_t* ast) {
    char filename[500] = "./Files/";
    prog_t* next_program;
    ast_t* next_ast;
    ast_node_t* ast_node;

    token_t* token = dequeueToken(program);

    /* FIXME be careful about strocn() filename twice */
    if (token->type == STRCON) {
        getSTRCON(token->attrib);
        strcat(filename, token->attrib);

        if (!getFilename(symbols, filename)) {
            next_ast = initAST();
            addFilename(symbols, filename, next_ast);

            ast_node = buildASTFile(getFilename(symbols, filename));
            addNode(ast, ast_node);

            next_program = tokenizeFile(filename, symbols);
            prog(next_program, symbols, ast);
            freeProgQueue(next_program);

            printf("Finished %s\n", filename);
        }

        instr(program, symbols, ast);

    } else {
        ERROR(token);
    }
}

/* Is this the right condition for prog_abort? */
void prog_abort(prog_t* program, symbol_t* symbols, ast_t* ast) {
    token_t* token = dequeueToken(program);

    ast_node_t* ast_node;

    if (token->type == SECTION && !strcmp(token->attrib, "}")) {
        ast_node = buildASTAbort();
        addNode(ast, ast_node);

        return;
    } else {
        ERROR(token);
    }
}

void in2str(prog_t* program, symbol_t* symbols, ast_t* ast) {
    token_t* token_string[TWO_ARG_LEN];

    if (parseBrackets(program, STRVAR, IN2STR_ARGS)) {
        instr(program, symbols, ast);

    } else {
        ERROR(peekToken(program, 0));
    }
}

void innum(prog_t* program, symbol_t* symbols, ast_t* ast) {
    if (parseBrackets(program, NUMVAR, INNUM_ARGS)) {
        instr(program, symbols, ast);

    } else {
        ERROR(peekToken(program, 0));
    }
}

/* FIXME ifequal and ifgreater are identical */
/* QUESTION Does this need two sets of instr for {} */
void ifequal(prog_t* program, symbol_t* symbols, ast_t* ast) {
    token_t* token = peekToken(program, 0);
    if (parseCondBracket(program)) {
        token = dequeueToken(program);

        /* Could just reuse prog(); */
        if (!strcmp(token->attrib, "{")) {
            instr(program, symbols, ast);
            printf("COND CONT\n");
            instr(program, symbols, ast);
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

void ifgreater(prog_t* program, symbol_t* symbols, ast_t* ast) {
    token_t* token = peekToken(program, 0);
    if (parseCondBracket(program)) {
        token = dequeueToken(program);

        if (!strcmp(token->attrib, "{")) {
            instr(program, symbols, ast);
            printf("COND CONT\n");
            instr(program, symbols, ast);
            return;
        } else {
            ERROR(token);
            return;
        }

    } else {
        ERROR(token);
    }
}

void inc(prog_t* program, symbol_t* symbols, ast_t* ast) {
    if (parseBrackets(program, NUMVAR, INC_ARGS)) {
        instr(program, symbols, ast);

    } else {
        ERROR(peekToken(program, 0));
    }
}

void jump(prog_t* program, symbol_t* symbols, ast_t* ast) {
    token_t* token = dequeueToken(program);
    ast_node_t* ast_node;

    if (token->type == NUMCON) {
        instr(program, symbols, ast);
    } else {
        ERROR(token);
    }
}

void print(prog_t* program, type_t type, symbol_t* symbols, ast_t* ast) {
    token_t* token = dequeueToken(program);
    ast_node_t* ast_node;

    if (token->type == STRVAR || token->type == NUMVAR ||
        token->type == STRCON || token->type == NUMCON) {
        ast_node = buildASTPrint(type, token->type, token->attrib);
        instr(program, symbols, ast);
    } else {
        ERROR(token);
    }
}

void rnd(prog_t* program, symbol_t* symbols, ast_t* ast) {
    if (parseBrackets(program, NUMVAR, RND_ARGS)) {
        instr(program, symbols, ast);

    } else {
        ERROR(peekToken(program, 0));
    }
}

void set(prog_t* program, type_t var, symbol_t* symbols, ast_t* ast) {
    token_t* token = dequeueToken(program);

    switch (var) {
        case STRVAR:
            if (token->type == SET) {
                token = dequeueToken(program);
                if (token->type == STRCON || token->type == STRVAR) {
                    instr(program, symbols, ast);
                    return;
                }
            }
            ERROR(token);
            break;
        case NUMVAR:
            if (token->type == SET) {
                token = dequeueToken(program);
                if (token->type == NUMVAR || token->type == NUMCON) {
                    instr(program, symbols, ast);
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

token_t* parseBracketsEdit(token_t* tokens[], type_t arg, int len) {
    int i;
    if (strcmp(tokens[0]->attrib, "(")) {
        return tokens[0];
    }

    for (i = 1; i < len - 2; i += 2) {
        if (tokens[i]->type != arg) {
            return tokens[i];
        }
        if (tokens[i + 1]->type != COMMA) {
            return tokens[i + 1];
        }
    }

    if (tokens[len - 2]->type != arg) {
        return tokens[len - 2];
    }
    if (strcmp(tokens[len - 1]->attrib, ")")) {
        return tokens[len - 1];
    }
    return NULL;
}

void fillTokenString(prog_t* program, token_t* tokens[], int len) {
    int i;
    for (i = 0; i < len; i++) {
        tokens[i] = dequeueToken(program);
    }
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
