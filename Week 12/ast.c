#include "ast.h"
#include <stdlib.h>

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
    tmp->data = NULL;
    return tmp;
}

ast_node_t* buildASTFile(mvmcell* file) {
    ast_node_t* tmp = initASTNode(FILE_);
    tmp->data->var = file;
    return tmp;
}

ast_node_t* buildASTAbort(void) {
    ast_node_t* tmp = initASTNode(ABORT);
    tmp->data->jump = NULL;
    return tmp;
}

ast_node_t* buildASTIN2STR(mvmcell* input1, mvmcell* input2) {
    ast_node_t* tmp = initASTNode(IN2STR);

    tmp->data->TWO_OPNode.op1 = initASTNode(STRVAR);
    tmp->data->TWO_OPNode.op2 = initASTNode(STRVAR);

    tmp->data->TWO_OPNode.op1->data->var = input1;
    tmp->data->TWO_OPNode.op2->data->var = input2;

    return tmp;
}

ast_node_t* buildASTINNUM(mvmcell* input) {
    ast_node_t* tmp = initASTNode(INNUM);

    tmp->data->ONE_OPNode.op = initASTNode(NUMVAR);
    tmp->data->ONE_OPNode.op->data->var = input;

    return tmp;
}

ast_node_t* buildASTJump(ast_node_t* jump) {
    ast_node_t* tmp = initASTNode(JUMP);
    tmp->data->jump = jump;

    return tmp;
}

ast_node_t* buildASTPrint(type_t print, type_t type, void* input) {
    ast_node_t* tmp = initASTNode(print);
    tmp->data->ONE_OPNode.op = initASTNode(type);

    if (type == STRVAR || type == NUMVAR) {
        tmp->data->ONE_OPNode.op->data->var = (mvmcell*)input;
    } else {
        tmp->data->ONE_OPNode.op->data->con = (char*)input;
    }

    return tmp;
}

ast_node_t* buildASTRnd(mvmcell* numvar) {
    ast_node_t* tmp = initASTNode(RND);
    tmp->data->ONE_OPNode.op = initASTNode(NUMVAR);
    tmp->data->ONE_OPNode.op->data->var = numvar;
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

    tmp->data->ONE_OPNode.op = initASTNode(NUMVAR);
    tmp->data->ONE_OPNode.op->data->var = numvar;

    return tmp;
}

ast_node_t* buildASTSet(type_t type1, type_t type2, mvmcell* var, void* varcon) {
    ast_node_t* tmp = initASTNode(SET);

    tmp->data->TWO_OPNode.op1 = initASTNode(type1);
    tmp->data->TWO_OPNode.op2 = initASTNode(type2);

    tmp->data->TWO_OPNode.op1->data->var = var;
    insertData(tmp->data->TWO_OPNode.op2, type2, varcon);

    return tmp;
}
