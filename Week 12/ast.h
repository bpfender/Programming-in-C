#ifndef AST_H
#define AST_H

#include "symbols.h"
#include "tokenizer.h"

typedef union syntax_node_t {
    char* con;

    mvmcell* var;

    ast_node_t* jump;

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

typedef struct ast_node_t {
    type_t type;
    syntax_node_t* data;
} ast_node_t;

typedef struct ast_t {
    syntax_node_t* head;
    syntax_node_t* curr;
} ast_t;

#endif
