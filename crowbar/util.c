/*
 * File : util.c
 * CreateDate : 2019-11-26 07:40:14
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static CRB_Interpreter *st_current_interpreter;

CRB_Interpreter *crb_get_current_interpreter(void)
{
    return st_current_interpreter;
}

void crb_set_current_interpreter(CRB_Interpreter *inter)
{
    st_current_interpreter = inter;
}

Variable* crb_search_global_variable(CRB_Interpreter *inter, char *identifier)
{
    Variable *pos;

    for (pos = inter->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier)) {
            return pos;
        }
    }

    return NULL;
}

Variable* crb_search_local_variable(LocalEnvironment *env, char *identifier)
{
    Variable *pos;
    if (NULL == env) {
        return NULL;
    }

    for (pos = env->variable; pos; pos = pos->next) {
        if (!strcmp(pos->name, identifier)) {
            return pos;
        }
    }

    return NULL;
}

char* crb_get_operator_string(ExpressionType type)
{
    char *str;
    switch (type) {
        case BOOLEAN_EXPRESSION:
        case INT_EXPRESSION:
        case DOUBLE_EXPRESSION:
        case STRING_EXPRESSION:
        case IDENTIFIER_EXPRESSION:
            DBG_panic(("bad expression type..%d\n", type));
            break;
        case ASSIGN_EXPRESSION:
            str = "=";
            break;
        case ADD_EXPRESSION:
            str = "+";
            break;
        case SUB_EXPRESSION:
            str = "-";
            break;
        case MUL_EXPRESSION:
            str = "*";
            break;
        case DIV_EXPRESSION:
            str = "/";
            break;
        case MOD_EXPRESSION:
            str = "%";
            break;
        case LOGICAL_AND_EXPRESSION:
            str = "&&";
            break;
        case LOGICAL_OR_EXPRESSION:
            str = "||";
            break;
        case EQ_EXPRESSION:
            str = "==";
            break;
        case NE_EXPRESSION:
            str = "!=";
            break;
        case GT_EXPRESSION:
            str = ">";
            break;
        case GE_EXPRESSION:
            str = ">=";
            break;
        case LT_EXPRESSION:
            str = "<";
            break;
        case LE_EXPRESSION:
            str = "<=";
            break;
        case MINUS_EXPRESSION:
            str = "-";
            break;
        case FUNCTION_CALL_EXPRESSION:
        case NULL_EXPRESSION:
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad expression type..%d\n", type));
    }

    return str;
}

FunctionDefinition *crb_search_function(char *name)
{
    FunctionDefinition *pos;
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();

    for (pos = inter->function_list; pos; pos = pos->next) {
        if (!strcmp(pos->name, name)) {
            break;
        }
    }
    return pos;
}

/* vim: set tabstop=4 set shiftwidth=4 */

