/*
 * File : create.c
 * CreateDate : 2019-11-23 22:31:28
 * */

#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

Expression* crb_create_index_expression(Expression *array, Expression *index)
{
    Expression *exp;

    exp = crb_alloc_expression(INDEX_EXPRESSION);
    exp->u.index_expression.array = array;
    exp->u.index_expression.index = index;

    return exp;
}

Expression* crb_create_method_call_expression(Expression *expression, char *method_name, ArgumentList *argument)
{
    Expression* exp;
    /* fprintf(stderr, "crb_create_method_call_expression method_name:%s\n", method_name); */

    exp = crb_alloc_expression(METHOD_CALL_EXPRESSION);
    exp->u.method_call_expression.expression = expression;
    exp->u.method_call_expression.identifier = method_name;
    exp->u.method_call_expression.argument = argument;

    return exp;
}

Expression* crb_create_incdec_expression(Expression *operand, ExpressionType inc_or_dec)
{
    Expression *exp;

    exp = crb_alloc_expression(inc_or_dec);
    exp->u.inc_dec.operand = operand;

    return exp;
}

Expression* crb_create_array_expression(ExpressionList *list)
{
    Expression *expr;

    expr = crb_alloc_expression(ARRAY_EXPRESSION);
    expr->u.array_literal = list;

    fprintf(stderr, "crb_create_array_expression -----%p\n", expr);
    return expr;
}

ExpressionList* crb_create_expression_list(Expression *expr)
{
    ExpressionList *el;

    el = crb_malloc(sizeof(ExpressionList));
    el->expression = expr;
    el->next = NULL;

    return el;
}

ExpressionList* crb_chain_expression_list(ExpressionList *list, Expression *expr)
{
    ExpressionList *pos;

    for (pos = list; pos->next; pos = pos->next) {
        ;
    }

    pos->next = crb_create_expression_list(expr);

    return list;
}

void crb_function_define(char *identifier, ParameterList *parameter_list, Block *block)
{
    FunctionDefinition *f;
    CRB_Interpreter *inter;

    if (crb_search_function(identifier))
    {
        crb_runtime_error(FUNCTION_MULTIOPLE_DEFINE_ERR, STRING_MESSAGE_ARGUMENT, "name", identifier, MESSAGE_ARGUMENT_END);
        return;
    }

    inter = crb_get_current_interpreter();

    f = crb_malloc(sizeof(FunctionDefinition));
    f->name = identifier;
    f->type = CROWBAR_FUNCTION_DEFINITION;
    f->u.crowbar_f.parameter = parameter_list;
    f->u.crowbar_f.block = block;
    /* 加到函数列表前面 */
    f->next = inter->function_list;
    inter->function_list = f;
}

ParameterList *crb_create_parameter(char *identifier)
{
    ParameterList *p;
    p = crb_malloc(sizeof(ParameterList));
    p->name = identifier;
    p->next = NULL;
    return p;
}

ParameterList *crb_chain_parameter(ParameterList *list, char *identifier)
{
    ParameterList *pos;
    for (pos = list; pos->next; pos = pos->next) {
        ;
    }

    pos->next = crb_create_parameter(identifier);
    return list;
}

ArgumentList *crb_create_argument_list(Expression *expression)
{
    ArgumentList *al;

    al = crb_malloc(sizeof(ArgumentList));
    al->expression = expression;
    al->next = NULL;

    return al;
}

ArgumentList *crb_chain_argument_list(ArgumentList *al, Expression *expr)
{
    ArgumentList *pos;
    for (pos = al; pos->next; pos = pos->next) {
        ;
    }

    pos->next = crb_create_argument_list(expr);
    return al;
}

StatementList* crb_create_statement_list(Statement *statement)
{
    StatementList *sl;
    /* fprintf(stderr, "crb_create_statement_list -- %d\n", statement->line_number); */
    sl = crb_malloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL;

    return sl;
}

StatementList* crb_chain_statement_list(StatementList *list, Statement *statement)
{
    StatementList *pos;
    /* fprintf(stderr, "crb_chain_statement_list ===\n"); */
    if(NULL == list) {
        return crb_create_statement_list(statement);
    }

    for (pos = list; pos->next; pos = pos->next) {
        ;
    }

    pos->next = crb_create_statement_list(statement);
    return list;
}

Expression* crb_alloc_expression(ExpressionType type)
{
    Expression *expr;

    expr = crb_malloc(sizeof(Expression));
    expr->type = type;
    expr->line_number = crb_get_current_interpreter()->current_line_number;

    return expr;
}

Expression* crb_create_assign_expression(Expression *left, Expression *operand)
{
    Expression *expr;

    expr = crb_alloc_expression(ASSIGN_EXPRESSION);
    expr->u.assign_expression.left = left;
    expr->u.assign_expression.operand = operand;

    return expr;
}

static Expression convert_value_to_expression(CRB_Value *v)
{
    Expression expr;

    if (CRB_INT_VALUE == v->type) {
        expr.type = INT_EXPRESSION;
        expr.u.int_value = v->u.int_value;
    } else if (CRB_DOUBLE_VALUE == v->type) {
        expr.type = DOUBLE_EXPRESSION;
        expr.u.int_value = v->u.double_value;
    } else {
        DBG_assert(CRB_BOOLEAN_VALUE == v->type, ("v->type:%d\n", v->type)); 
        /* ((CRB_BOOLEAN_VALUE == v->type) ? (void)(0) : ((DBG_set(dbg_default_controller, "create.c", 133)), (DBG_set_expression("CRB_BOOLEAN_VALUE == v->type")), DBG_assert_func ("v->type:%d\n", v->type))); */
        expr.type = BOOLEAN_EXPRESSION;
        expr.u.boolean_value = v->u.boolean_value;
    }

    return expr;
}

Expression* crb_create_binary_expression(ExpressionType operator, Expression *left, Expression *right)
{
   if ( ( (INT_EXPRESSION == left->type) || (DOUBLE_EXPRESSION == left->type) ) &&
           ( (INT_EXPRESSION== right->type || DOUBLE_EXPRESSION == right->type) ) ) {
        CRB_Value v;
        v = crb_eval_binary_expression(crb_get_current_interpreter(), NULL, operator, left, right);
        *left = convert_value_to_expression(&v);
        return left;
    } else {
        Expression *expr;
        expr = crb_alloc_expression(operator);
        expr->u.binary_expression.left = left;
        expr->u.binary_expression.right = right;
        return expr;
    }
}

Expression* crb_create_minus_expression(Expression *operand)
{
    if (INT_EXPRESSION == operand->type || DOUBLE_EXPRESSION == operand->type) {
        CRB_Value v;
        v = crb_eval_minus_expression(crb_get_current_interpreter(), NULL, operand);
        *operand = convert_value_to_expression(&v);
        return operand;
    } else {
        Expression *expr;
        expr = crb_alloc_expression(MINUS_EXPRESSION);
        expr->u.minus_expression = operand;
        return expr;
    }
}

Expression* crb_create_identifier_expression(char *identifier)
{
    Expression *expr;
    expr = crb_alloc_expression(IDENTIFIER_EXPRESSION);
    expr->u.identifier = identifier;
    /* fprintf(stderr, "crb_create_identifier_expression identifier:%s %d\n", identifier, expr->line_number); */

    return expr;
}

Expression* crb_create_function_call_expression(char *func_name, ArgumentList *argument)
{
    Expression *expr;
    /* fprintf(stderr, "crb_create_function_call_expression func_name:%s\n", func_name); */
    expr= crb_alloc_expression(FUNCTION_CALL_EXPRESSION);
    expr->u.function_call_expression.identifier = func_name;
    expr->u.function_call_expression.argument = argument;
    return expr;
}

Expression* crb_create_boolean_expression(CRB_Boolean value)
{
    Expression *expr;
    expr = crb_alloc_expression(BOOLEAN_EXPRESSION);
    expr->u.boolean_value = value;

    return expr;
}

Expression* crb_create_null_expression(void)
{
    Expression *expr;
    expr = crb_alloc_expression(NULL_EXPRESSION);

    return expr;
}

static Statement* alloc_statement(StatementType type)
{
    Statement *st;
    st = crb_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = crb_get_current_interpreter()->current_line_number;

    return st;
}

Statement* crb_create_global_statement(IdentifierList *identifier_list)
{
    Statement *st;

    /* fprintf(stderr, "crb_create_global_statement identifier:%s\n", identifier_list->name); */
    st = alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;
    return st;
}

IdentifierList* crb_create_global_identifier(char *identifier)
{
    IdentifierList *list;
    /* fprintf(stderr, "crb_create_global_identifier ~~~~~~ identifier:%s\n", identifier); */
    list = crb_malloc(sizeof(IdentifierList));
    list->name = identifier;
    list->next = NULL;

    return list;
}

IdentifierList* crb_chain_identifier(IdentifierList* list, char *identifier)
{
    IdentifierList *pos;

    for (pos = list; pos->next; pos = pos->next) {
        ;
    }

    pos->next = crb_create_global_identifier(identifier);

    return list;
}

Statement* crb_create_if_statement(Expression *cond, Block *then_block, Elsif *elsif_block, Block *else_block)
{
    Statement *st;
    st = alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = cond;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elsif_list = elsif_block;
    st->u.if_s.else_block = else_block;

    return st;
}

Elsif* crb_chain_elsif_list(Elsif *list, Elsif *add)
{
    Elsif *pos;

    for (pos = list; pos->next; pos = pos->next) {
        ;
    }

    pos->next = add;

    return list;
}

Elsif* crb_create_elsif(Expression *expr, Block *block)
{
    Elsif *ei;
    ei = crb_malloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;

    return ei;
}

Statement* crb_create_while_statement(Expression *cond, Block *block)
{
    Statement *st;
    st = alloc_statement(WHILE_STATEMENT);
    st->u.while_s.condition = cond;
    st->u.while_s.block = block;

    return st;
}

Statement* crb_create_for_statement(Expression *init, Expression *cond, Expression *post, Block *block)
{
    Statement *st;

    st = alloc_statement(FOR_STATEMENT);
    /* fprintf(stderr, "|||| eval_expression left:%d op:%d\n", init->u.assign_expression.left->type , init->u.assign_expression.operand->type); */
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;

    return st;
}

Block* crb_create_block(StatementList *statement_list)
{
    Block *block;
    block = crb_malloc(sizeof(Block));
    block->statement_list = statement_list;

    return block;
}

/* 表达式语句 */
Statement* crb_create_expression_statement(Expression *expr)
{
    Statement *st;
    st = alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expr;

    return st;
}

Statement* crb_create_return_statement(Expression *expr)
{
    Statement *st;
    st = alloc_statement(RETURN_STATEMENT);
    st->u.return_s.return_value = expr;

    return st;
}

Statement *crb_create_break_statement(void)
{
    return alloc_statement(BREAK_STATEMENT);
}

Statement *crb_create_continue_statement(void)
{
    return alloc_statement(CONTINUE_STATEMENT);
}

/* vim: set tabstop=4 set shiftwidth=4 */

