/*
 * File : execute.c
 * CreateDate : 2019-11-27 08:50:41
 * */

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static StatementResult execute_expression_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    CRB_Value v;

    result.type = NORMAL_STATEMENT_RESULT;
    v = crb_eval_expression(inter, env, statement->u.expression_s);
    if (CRB_STRING_VALUE == v.type) {
        crb_release_string(v.u.string_value);
    }

    return result;
}

static StatementResult execute_statement(CRB_Interpreter *inter, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    switch (statement->type){
    case EXPRESSION_STATEMENT:
        result = execute_expression_statement(inter, env, statement);
        break;
    case GLOBAL_STATEMENT:
        break;
    case IF_STATEMENT:
        break;
    case WHILE_STATEMENT:
        break;
    case FOR_STATEMENT:
        break;
    case RETURN_STATEMENT:
        break;
    case BREAK_STATEMENT:
        break;
    case CONTINUE_STATEMENT:
        break;
    case STATEMENT_TYPE_COUNT_PLUS_1:
    default:
        DBG_panic(("bad case...%d", statement->type));
    }

    return result;
}

StatementResult crb_execute_statement_list(CRB_Interpreter *inter, LocalEnvironment *env, StatementList *list)
{
    StatementList *pos;
    StatementResult result;

    result.type = NORMAL_STATEMENT_RESULT;

    for (pos = list; pos; pos = pos->next) {
        result = execute_statement(inter, env, pos->statement);
        if (result.type != NORMAL_STATEMENT_RESULT) {
            goto FUNC_END;
        }
    }
FUNC_END:
    return result;
}

/* vim: set tabstop=4 set shiftwidth=4 */

