#include "ast.h"
#include <stdlib.h>

/* FIXME filename array is a bit clunky */
ast_t* initAST(void) {
    ast_t* tmp = (ast_t*)malloc(sizeof(ast_t));
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }
    tmp->curr = NULL;
    tmp->head = NULL;

    return tmp;
}

ast_node_t* initASTNode(type_t type) {
    ast_node_t* tmp = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!tmp) {
        ON_ERROR("Memory allocation error\n");
    }

    tmp->type = type;

    tmp->data = (syntax_node_t*)malloc(sizeof(syntax_node_t));
    if (!tmp->data) {
        ON_ERROR("Memory allocation error\n");
    }

    return tmp;
}

ast_node_t* buildASTFile(mvmcell* file) {
    ast_node_t* tmp = initASTNode(FILE_);

    tmp->data->FILE_Node.file = file;
    return tmp;
}

ast_node_t* buildASTAbort(void) {
    ast_node_t* tmp = initASTNode(ABORT);

    tmp->data->ONE_OP_Node.op = NULL;
    return tmp;
}

ast_node_t* buildASTIN2STR(mvmcell* input1, mvmcell* input2) {
    ast_node_t* tmp = initASTNode(IN2STR);

    tmp->data->TWO_OP_Node.op1 = initASTNode(STRVAR);
    tmp->data->TWO_OP_Node.op2 = initASTNode(STRVAR);

    tmp->data->TWO_OP_Node.op1->data->var = input1;
    tmp->data->TWO_OP_Node.op2->data->var = input2;

    return tmp;
}

ast_node_t* buildASTINNUM(mvmcell* input) {
    ast_node_t* tmp = initASTNode(INNUM);

    tmp->data->ONE_OP_Node.op = initASTNode(NUMVAR);
    tmp->data->ONE_OP_Node.op->data->var = input;

    return tmp;
}

ast_node_t* buildASTJump(ast_node_t* jump) {
    ast_node_t* tmp = initASTNode(JUMP);

    tmp->data->ONE_OP_Node.op = jump;
    return tmp;
}

ast_node_t* buildASTPrint(type_t print, type_t type, void* input) {
    ast_node_t* tmp = initASTNode(print);

    tmp->data->ONE_OP_Node.op = initASTNode(type);
    insertData(tmp->data->ONE_OP_Node.op, type, input);

    return tmp;
}

ast_node_t* buildASTRnd(mvmcell* numvar) {
    ast_node_t* tmp = initASTNode(RND);

    tmp->data->ONE_OP_Node.op = initASTNode(NUMVAR);
    tmp->data->ONE_OP_Node.op->data->var = numvar;
    return tmp;
}

ast_node_t* buildASTCond(type_t cond, type_t type1, type_t type2, void* input1, void* input2) {
    ast_node_t* tmp = initASTNode(cond);

    tmp->data->CONDNode.op1 = initASTNode(type1);
    tmp->data->CONDNode.op2 = initASTNode(type2);

    insertData(tmp->data->CONDNode.op1, type1, input1);
    insertData(tmp->data->CONDNode.op2, type2, input2);

    return tmp;
}

void insertData(ast_node_t* node, type_t type, void* input) {
    if (type == STRVAR || type == NUMVAR) {
        node->data->var = (mvmcell*)input;
    } else {
        node->data->con = (char*)input;
    }
}

ast_node_t* buildASTInc(mvmcell* numvar) {
    ast_node_t* tmp = initASTNode(INC);

    tmp->data->ONE_OP_Node.op = initASTNode(NUMVAR);
    tmp->data->ONE_OP_Node.op->data->var = numvar;
    return tmp;
}

ast_node_t* buildASTSet(type_t type1, type_t type2, mvmcell* var, void* varcon) {
    ast_node_t* tmp = initASTNode(SET);

    tmp->data->TWO_OP_Node.op1 = initASTNode(type1);
    tmp->data->TWO_OP_Node.op2 = initASTNode(type2);

    tmp->data->TWO_OP_Node.op1->data->var = var;
    insertData(tmp->data->TWO_OP_Node.op2, type2, varcon);

    return tmp;
}

void addNode(ast_t* ast, ast_node_t* node) {
    if (ast->curr) {
        switch (ast->curr->type) {
            case FILE_:
            case ABORT:
            case INNUM:
            case JUMP:
            case PRINT:
            case PRINTN:
            case RND:
            case INC:
                ast->curr->data->ONE_OP_Node.next = node;
                break;
            case SET:
            case IN2STR:
                ast->curr->data->TWO_OP_Node.next = node;
                break;
            case IFEQUAL:
            case IFGREATER:
                ast->curr->data->CONDNode.next = node;
                break;
            default:
                exit(EXIT_FAILURE);
        }
        ast->curr = node;

    } else {
        ast->curr = ast->head = node;
    }
}

