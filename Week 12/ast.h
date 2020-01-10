#ifndef AST_H
#define AST_H

#include <string.h>
#include "symbols.h"
#include "tokenizer.h"

typedef struct ast_node_t ast_node_t;

typedef union syntax_node_t {
    char* con;

    mvmcell* var;

    struct ast_node_t* jump;

    struct {
        struct ast_node_t* op;
        struct ast_node_t* next;

    } ONE_OPNode;

    struct {
        struct ast_node_t* op1;
        struct ast_node_t* op2;
        struct ast_node_t* next;
    } TWO_OPNode;

    struct {
        struct ast_node_t* op1;
        struct ast_node_t* op2;
        struct ast_node_t* next;
        struct ast_node_t* skip;
    } CONDNode;
} syntax_node_t;

struct ast_node_t {
    type_t type;
    syntax_node_t* data;
};

typedef struct ast_t {
    ast_node_t* head;
    ast_node_t* curr;
} ast_t;

ast_t* initAST(void);
ast_node_t* initASTNode(type_t type);
ast_node_t* buildASTFile(mvmcell* file);
ast_node_t* buildASTAbort(void);
ast_node_t* buildASTIN2STR(mvmcell* input1, mvmcell* input2);
ast_node_t* buildASTINNUM(mvmcell* input);
ast_node_t* buildASTJump(ast_node_t* jump);
ast_node_t* buildASTPrint(type_t print, type_t type, void* input);
ast_node_t* buildASTRnd(mvmcell* numvar);
ast_node_t* buildASTCond(type_t cond, type_t type1, type_t type2, void* input1, void* input2);
ast_node_t* buildASTInc(mvmcell* numvar);
ast_node_t* buildASTSet(type_t type1, type_t type2, mvmcell* var, void* varcon);

void insertData(ast_node_t* node, type_t type, void* input);
void addNode(ast_t* ast, ast_node_t* node);

#endif
