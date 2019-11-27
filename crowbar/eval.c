/*
 * File : evacl.c
 * CreateDate : 2019-11-27 12:22:57
 * */

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static CRB_Value eval_boolean_expression(CRB_Boolean boolean_value)
{
    CRB_Value v;
    v.type = CRB_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;

    return v;
}

static CRB_Value eval_int_expression(int int_value)
{
    CRB_Value v;
    v.type = CRB_INT_VALUE;
    v.u.int_value = int_value;

    return v;
}

static CRB_Value eval_double_expression(double double_value)
{
    CRB_Value v;
    v.type = CRB_DOUBLE_VALUE;
    v.u.double_value = double_value;

    return v;
}

static CRB_Value eval_string_expression(CRB_Interpreter *inter, char *string_value)
{
    CRB_Value v;
    v.type = CRB_STRING_VALUE;
    v.u.string_value = crb_literal_to_crb_string(inter, string_value);

    return v;
}

static CRB_Value eval_null_expression(void)
{
    CRB_Value v;
    v.type = CRB_NULL_VALUE;

    return v;
}

static void refer_if_string(CRB_Value *v)
{
    if (CRB_STRING_VALUE == v->type) {
        crb_refer_string(v->u.string_value);
    }
}

static void release_if_string(CRB_Value *v)
{
    if (CRB_STRING_VALUE == v->type) {
        crb_release_string(v->u.string_value);
    }
}

static Variable *search_global_variable_from_env(CRB_Interpreter *inter, LocalEnvironment *env, char *name)
{
    GlobalVariableRef *pos;

    if (NULL == env) {
        return crb_search_global_variable(inter, name);
    }

    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->variable->name, name)) {
            return pos->variable;
        }
    }

    return NULL;
}

static CRB_Value eval_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr)
{
    CRB_Value v;

    switch (expr->type) {
        case BOOLEAN_EXPRESSION:
            eval_boolean_expression(expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
            eval_int_expression(expr->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            eval_double_expression(expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            eval_string_expression(inter, expr->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            eval_identifier_expression(inter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            eval_assign_expression(inter, env,
                    expr->u.assign_expression.variable,
                    expr->u.assign_expression.operand);
            break;
        case ADD_EXPRESSION:
        case SUB_EXPRESSION:
        case MUL_EXPRESSION:
        case DIV_EXPRESSION:
        case MOD_EXPRESSION:
        case EQ_EXPRESSION:
        case NE_EXPRESSION:
        case GT_EXPRESSION:
        case GE_EXPRESSION:
        case LT_EXPRESSION:
        case LE_EXPRESSION:
            v = crb_eval_binary_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;

        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            v = eval_logical_and_or_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;

        case MINUS_EXPRESSION:
            v = crb_eval_minus_expression(inter, env, expr->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            v = eval_function_call_expression(inter, env, expr);
            break;
        case NULL_EXPRESSION:
            v = eval_null_expression();
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic("bad case. type:%d\n", expr->type);
    }

    return v;
}


CRB_Value crb_eval_expression(CRB_Interpreter *inter, LocalEnvironment *env, Expression *expr)
{
    return eval_expression(inter, env, expr);
}

/* vim: set tabstop=4 set shiftwidth=4 */

