/*
 * File : evacl.c
 * CreateDate : 2019-11-27 12:22:57
 * */

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr);

static void eval_boolean_expression(CRB_Interpreter *inter, CRB_Boolean boolean_value)
{
    CRB_Value v;
    v.type = CRB_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;

    push_value(inter, &v);
}


static void eval_int_expression(CRB_Interpreter *inter, int int_value)
{
    CRB_Value v;
    v.type = CRB_INT_VALUE;
    v.u.int_value = int_value;

    /* fprintf(stderr, "eval_int_expression %d\n", int_value); */
    push_value(inter, &v);
}

static void eval_double_expression(CRB_Interpreter *inter, double double_value)
{
    CRB_Value v;
    v.type = CRB_DOUBLE_VALUE;
    v.u.double_value = double_value;

    push_value(inter, &v);
}

static void eval_string_expression(CRB_Interpreter *inter, char *string_value)
{
    CRB_Value v;
    v.type = CRB_STRING_VALUE;
    /* fprintf(stderr, "eval_string_expression %s|\n", string_value); */
    v.u.object = crb_literal_to_crb_string(inter, string_value);

    push_value(inter, &v);
}

static void eval_null_expression(CRB_Interpreter *inter)
{
    CRB_Value v;
    v.type = CRB_NULL_VALUE;

    push_value(inter, &v);
}



static Variable *search_global_variable_from_env(CRB_Interpreter *inter, CRB_LocalEnvironment *env, char *name)
{
    GlobalVariableRef *pos;

    if (NULL == env) {
        return crb_search_global_variable(inter, name);
    }

    /* fprintf(stderr, "search_global_variable_from_env env->global_variable:%p\n", env->global_variable);  */
    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->variable->name, name)) {
            return pos->variable;
        }
    }

    return NULL;
}

static void eval_identifier_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value v;
    Variable *vp;

    vp = crb_search_local_variable(env, expr->u.identifier);
    /* fprintf(stderr, "eval_identifier_expression crb_search_local_variable vup:%p line:%d identifier:%s\n", vp, expr->line_number, expr->u.identifier); */
    if (vp != NULL) {
        v = vp->value;
        push_value(inter, &v);
        return;
    }

    vp = search_global_variable_from_env(inter, env, expr->u.identifier);
    /* fprintf(stderr, "eval_identifier_expression search_global_variable_from_env vup:%p env:%p\n", vp, env); */
    if (vp != NULL) {
        v = vp->value;
    } else {
        /* fprintf(stderr, "eval_identifier_expression search_global_variable_from_env else vp:%p\n", vp); */
        crb_runtime_error(expr->line_number, VARIABLE_NOT_FOUND_ERR, STRING_MESSAGE_ARGUMENT, "token", expr->u.identifier, MESSAGE_ARGUMENT_END);
    }

    push_value(inter, &v);
}

static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr);

static CRB_Value* get_identifier_value(CRB_Interpreter *inter, CRB_LocalEnvironment *env, char *identifier)
{
    Variable *new_var;
    Variable *left;
    /* fprintf(stderr, "@@@@get_identifier_value --> %s\n", identifier); */

    left = crb_search_local_variable(env, identifier);
    /* fprintf(stderr, "@@@@get_identifier_value --> %s crb_search_local_variable:%p env:%p\n", identifier, left, env); */
    if (NULL == left) {
        left = search_global_variable_from_env(inter, env, identifier);
    }
    /* fprintf(stderr, "@@@@get_identifier_value --> %s search_global_variable_from_env:%p env:%p\n", identifier, left, env); */

    if (left != NULL) {
        return &left->value;
    }

    if (env != NULL) {
        new_var = crb_add_local_variable(env, identifier);
    } else {
        new_var = crb_add_global_variable(inter, identifier);
    }
    left = new_var;

    return &left->value;
}

CRB_Value* get_array_element_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value array;
    CRB_Value index;

    fprintf(stderr,"get_array_element_lvalue eval_expression(env:%p array expr:%p)\n", env, expr->u.index_expression.array);
    eval_expression(inter, env, expr->u.index_expression.array);
    fprintf(stderr,"get_array_element_lvalue eval_expression(env:%p index expr:%p)\n", env, expr->u.index_expression.index);
    eval_expression(inter, env, expr->u.index_expression.index);

    index = pop_value(inter);
    array = pop_value(inter);

    if (array.type != CRB_ARRAY_VALUE) {
        crb_runtime_error(expr->line_number, INDEX_OPERAND_NOT_ARRAY_ERR, MESSAGE_ARGUMENT_END);
    }

    if (index.type != CRB_INT_VALUE) {
        crb_runtime_error(expr->line_number, INDEX_OPERAND_NOT_INT_ERR, MESSAGE_ARGUMENT_END);
    }

    if (index.u.int_value < 0 ||
            index.u.int_value >= array.u.object->u.array.size) {
        crb_runtime_error(expr->line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR, INT_MESSAGE_ARGUMENT, "size", array.u.object->u.array.size,
                INT_MESSAGE_ARGUMENT, "index", index.u.int_value, MESSAGE_ARGUMENT_END);
    }

    return &array.u.object->u.array.array[index.u.int_value];
}

CRB_Value* get_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *dest;
    /* fprintf(stderr, "get_lvalue start %d...\n", expr->type); */
    if (IDENTIFIER_EXPRESSION == expr->type) {
        dest = get_identifier_value(inter, env, expr->u.identifier);
    } else if (INDEX_EXPRESSION == expr->type) {
        dest = get_array_element_lvalue(inter, env, expr);
    } else {
        crb_runtime_error(expr->line_number, NOT_LVALUE_ERROR, MESSAGE_ARGUMENT_END);
    }

    return dest;
}

static void eval_assign_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *left, Expression *expr)
{
    CRB_Value *src;
    CRB_Value *dest;

    /* fprintf(stderr, "eval_assign_expression left:%d... expr:%d\n", left->type, expr->type); */ 
    fprintf(stderr, "eval_assign_expression eval_expression(env:%p left:%p %s expr:%p)\n", env, left, left->u.identifier, expr);
    eval_expression(inter, env, expr);
    src = peek_stack(inter, 0);
    /* fprintf(stderr, "eval_assign_expression peek_stack ok\n"); */

    dest = get_lvalue(inter, env, left);
    /* fprintf(stderr, "eval_assign_expression get_lvalue ok\n"); */
    *dest = *src;
}

static CRB_Boolean eval_binary_boolean(CRB_Interpreter *inter, ExpressionType operator, CRB_Boolean left, CRB_Boolean right, int line_number)
{
    CRB_Boolean result;
    if (EQ_EXPRESSION == operator) {
        result = left == right;
    } else if (NE_EXPRESSION == operator) {
        result = left != right;
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, NOT_BOOLEAN_OPERATOR_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }

    return result;
}

static void eval_binary_int(CRB_Interpreter *inter, ExpressionType operator, int left, int right, CRB_Value *result, int line_number)
{
    if (dkc_is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    } else if (dkc_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d line:%d\n", operator, line_number));
    }

    switch (operator) {
        case BOOLEAN_EXPRESSION:
        case INT_EXPRESSION:
        case DOUBLE_EXPRESSION:
        case STRING_EXPRESSION:
        case IDENTIFIER_EXPRESSION:
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case ..%d line:%d\n", operator, line_number));
            break;
        case ADD_EXPRESSION:
            result->u.int_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.int_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.int_value = left * right;
            break;
        case DIV_EXPRESSION:
            /* 0 */
            result->u.int_value = left / right;
            break;
        case MOD_EXPRESSION:
            /* 0 */
            result->u.int_value = left % right;
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            DBG_panic(("bad case ...%d line:%d\n", operator, line_number));
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = left <= right;
            break;
        case MINUS_EXPRESSION:
        case FUNCTION_CALL_EXPRESSION:
        case NULL_EXPRESSION:
        case ARRAY_EXPRESSION:
        case INDEX_EXPRESSION:
        case INCREMENT_EXPRESSION:
        case DECREMENT_EXPRESSION:
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case...%d line:%d\n", operator, line_number));
    }
}

static void eval_binary_double(CRB_Interpreter *inter, ExpressionType operator, double left, double right, CRB_Value *result, int line_number)
{
    if (dkc_is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    } else if (dkc_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_panic(("operator..%d line:%d\n", operator, line_number));
    }

    switch (operator) {
        case BOOLEAN_EXPRESSION:
        case INT_EXPRESSION:
        case DOUBLE_EXPRESSION:
        case STRING_EXPRESSION:
        case IDENTIFIER_EXPRESSION:
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case ..%d line:%d\n", operator, line_number));
            break;
        case ADD_EXPRESSION:
            result->u.double_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.double_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.double_value = left * right;
            break;
        case DIV_EXPRESSION:
            /* 0 */
            result->u.double_value = left / right;
            break;
        case MOD_EXPRESSION:
            /* 0 */
            result->u.double_value = fmod(left , right);
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            DBG_panic(("bad case ...%d line:%d\n", operator, line_number));
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = left <= right;
            break;
        case MINUS_EXPRESSION:
        case FUNCTION_CALL_EXPRESSION:
        case NULL_EXPRESSION:
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case...%d line:%d\n", operator, line_number));
    }
}

static CRB_Boolean eval_compare_string(ExpressionType operator, CRB_Value *left, CRB_Value *right, int line_number)
{
    CRB_Boolean result;
    int cmp;

    cmp = strcmp(left->u.object->u.string.string, right->u.object->u.string.string);

    if (EQ_EXPRESSION == operator) {
        result = (cmp == 0);
    } else if (NE_EXPRESSION == operator) {
        result = (cmp != 0);
    } else if (GT_EXPRESSION == operator) {
        result = (cmp > 0);
    } else if (GE_EXPRESSION == operator) {
        result = (cmp >= 0);
    } else if (LT_EXPRESSION == operator) {
        result = (cmp < 0);
    } else if (LE_EXPRESSION == operator) {
        result = (cmp <= 0);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, BAD_OPERATOR_FOR_STRING_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }

    return result;
}

static CRB_Boolean eval_binary_null(CRB_Interpreter *inter, ExpressionType operator, CRB_Value *left, CRB_Value *right, int line_number)
{
    CRB_Boolean result;

    if (EQ_EXPRESSION == operator) {
        result = (CRB_NULL_VALUE == left->type && CRB_NULL_VALUE == right->type);
    } else if (NE_EXPRESSION == operator) {
        result = !(CRB_NULL_VALUE == left->type && CRB_NULL_VALUE == right->type);
    } else {
        char *op_str = crb_get_operator_string(operator);
        crb_runtime_error(line_number, BAD_OPERATOR_FOR_STRING_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }

    return result;
}

void chain_string(CRB_Interpreter *inter, CRB_Value *left, CRB_Value *right, CRB_Value *result)
{
    int len;
    char *str;
    char *right_str;
    CRB_Object *right_obj;

    right_str = CRB_value_to_string(right);

    right_obj = crb_create_crowbar_string_i(inter, right_str);

    result->type = CRB_STRING_VALUE;
    len = strlen(left->u.object->u.string.string) + strlen(right_obj->u.string.string);
    str = MEM_malloc(len + 1);

    strcpy(str, left->u.object->u.string.string);
    strcat(str, right_obj->u.string.string);
    result->u.object = crb_create_crowbar_string_i(inter, str);
}


void eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right)
{
    CRB_Value *left_val;
    CRB_Value *right_val;
    CRB_Value result;

    fprintf(stderr, "eval_binary_expression eval_expression(env:%p left:%p)\n", env, left);
    eval_expression(inter, env, left);
    fprintf(stderr, "eval_binary_expression eval_expression(env:%p right:%p)\n", env, right);
    eval_expression(inter, env, right);
    left_val = peek_stack(inter, 1);
    right_val = peek_stack(inter, 0);

    if (CRB_INT_VALUE == left_val->type && CRB_INT_VALUE == right_val->type) {
        eval_binary_int(inter, operator, left_val->u.int_value, right_val->u.int_value, &result, left->line_number);

    } else if (CRB_DOUBLE_VALUE == left_val->type && CRB_DOUBLE_VALUE == right_val->type) {
        eval_binary_double(inter, operator, left_val->u.double_value, right_val->u.double_value, &result, left->line_number);

    } else if (CRB_INT_VALUE == left_val->type && CRB_DOUBLE_VALUE == right_val->type) {
        left_val->u.double_value = left_val->u.int_value;
        eval_binary_double(inter, operator, left_val->u.double_value, right_val->u.double_value, &result, left->line_number);

    } else if (CRB_DOUBLE_VALUE == left_val->type && CRB_INT_VALUE == right_val->type) {
        right_val->u.double_value = right_val->u.int_value;
        eval_binary_double(inter, operator, left_val->u.double_value, right_val->u.double_value, &result, left->line_number);

    } else if (CRB_BOOLEAN_VALUE == left_val->type && CRB_BOOLEAN_VALUE == right_val->type) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_boolean(inter, operator, left_val->u.boolean_value, right_val->u.boolean_value, left->line_number);

    } else if (CRB_STRING_VALUE == left_val->type && operator == ADD_EXPRESSION) {
        chain_string(inter, left_val, right_val, &result);

    } else if (CRB_STRING_VALUE == left_val->type && CRB_STRING_VALUE == right_val->type) { /* string 比较 */
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_compare_string(operator, left_val, right_val, left->line_number);

    } else if (CRB_NULL_VALUE == left_val->type || CRB_NULL_VALUE == right_val->type) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_null(inter, operator, left_val, right_val, left->line_number);

    } else {
        char *op_str = crb_get_operator_string(operator);
        /* fprintf(stderr, "eval_binary_expression runtime error left:%d right:%d\n", left_val->type, right_val->type); */
        crb_runtime_error(left->line_number, BAD_OPERATOR_FOR_STRING_ERR, STRING_MESSAGE_ARGUMENT, "operator", op_str, MESSAGE_ARGUMENT_END);
    }

    pop_value(inter);
    pop_value(inter);
    /* fprintf(stderr, "eval_binary_expression result:%d\n", result.type); */
    push_value(inter, &result);
}

CRB_Value crb_eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right)
{
    eval_binary_expression(inter, env, operator, left, right);

    return pop_value(inter);
}

static CRB_Value eval_logical_and_or_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right)
{
    CRB_Value left_val;
    CRB_Value right_val;
    CRB_Value result;

    result.type = CRB_BOOLEAN_VALUE;
    eval_expression(inter, env, left);
    left_val = pop_value(inter);

    if (left_val.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(left->line_number, NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }

    if (LOGICAL_AND_EXPRESSION == operator) {
        if (!left_val.u.boolean_value) {
            result.u.boolean_value = CRB_FALSE;
            goto FUNC_END;
        }
    } else if (LOGICAL_OR_EXPRESSION == operator) {
        if (left_val.u.boolean_value) {
            result.u.boolean_value = CRB_TRUE;
            goto FUNC_END;
        }
    } else {
        DBG_panic(("bad operator..%d line:%d\n", operator, left->line_number));
    }

    eval_expression(inter, env, right);
    right_val = pop_value(inter);
    if (right_val.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(right->line_number, NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    result.u.boolean_value = right_val.u.boolean_value;

FUNC_END:
    push_value(inter, &result);
}

CRB_Value crb_eval_minus_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *operand)
{
    CRB_Value operand_val;
    CRB_Value result;

    eval_expression(inter, env, operand);
    operand_val = pop_value(inter);
    if(CRB_INT_VALUE == operand_val.type) {
        result.type = CRB_INT_VALUE;
        result.u.int_value = -operand_val.u.int_value;
    } else if (CRB_DOUBLE_VALUE == operand_val.type) {
        result.type = CRB_DOUBLE_VALUE;
        result.u.double_value = -operand_val.u.double_value;
    } else {
        crb_runtime_error(operand->line_number, MINUS_OPERAND_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }

    push_value(inter, &result);
}

static CRB_LocalEnvironment* alloc_local_environment(CRB_Interpreter *inter)
{
    CRB_LocalEnvironment *ret;

    ret = MEM_malloc(sizeof(CRB_LocalEnvironment));
    ret->variable = NULL;
    ret->global_variable = NULL;
    ret->ref_in_native_method = NULL;

    ret->next = inter->top_environment;
    inter->top_environment = ret;

    return ret;
}

void dispose_ref_in_native_method(CRB_LocalEnvironment *env)
{
    RefInNativeFunc *ref;

    while(env->ref_in_native_method) {
        ref = env->ref_in_native_method;
        env->ref_in_native_method = ref->next;
        /* 此处崩溃 new_array */
        MEM_free(ref);
    }
}

static void dispose_local_environment(CRB_Interpreter *inter)
{
    CRB_LocalEnvironment *env = inter->top_environment;

    while(env->variable) {
        Variable *tmp;
        tmp = env->variable;
        env->variable = tmp->next;
        MEM_free(tmp);
    }

    while(env->global_variable) {
        GlobalVariableRef *ref;
        ref = env->global_variable;
        env->global_variable = ref->next;
        MEM_free(ref);
    }

    dispose_ref_in_native_method(env);
    inter->top_environment = env->next;

    MEM_free(env);
}

static CRB_Value call_native_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_LocalEnvironment *caller_env, Expression *expr, CRB_NativeFunctionProc *proc)
{
    CRB_Value value;
    int arg_count;
    ArgumentList *arg_p;
    CRB_Value *args;

    for (arg_count = 0, arg_p = expr->u.function_call_expression.argument; arg_p; arg_p = arg_p->next) {
        fprintf(stderr, "call_native_function eval_expression(env:%p expr:%p)\n", caller_env, arg_p->expression);
        eval_expression(inter, caller_env, arg_p->expression);
        arg_count++;
    }

    args = &inter->stack.stack[inter->stack.stack_pointer - arg_count];

    value = proc(inter, env, arg_count, args);
    shrink_stack(inter, arg_count);

    push_value(inter, &value);
}

static void call_crowbar_function(CRB_Interpreter *inter, CRB_LocalEnvironment *local_env, CRB_LocalEnvironment *caller_env, Expression *expr, FunctionDefinition *func)
{
    CRB_Value v;
    StatementResult result;
    ArgumentList *arg_p;
    ParameterList *param_p;


    for (arg_p = expr->u.function_call_expression.argument, param_p = func->u.crowbar_f.parameter; arg_p; arg_p = arg_p->next, param_p = param_p->next) {

        CRB_Value arg_val;
        Variable *new_var;

        if (NULL == param_p) {
            crb_runtime_error(expr->line_number, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
        }

        fprintf(stderr,"call_crowbar_function eval_expression(env:%p expr:%p)\n", caller_env, arg_p->expression);
        eval_expression(inter, caller_env, arg_p->expression);
        arg_val = pop_value(inter);

        new_var = crb_add_local_variable(local_env, param_p->name); /* 添加到局部变量中 */
        new_var->value = arg_val;
    }

    if (param_p) {
        crb_runtime_error(expr->line_number, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }

    result = crb_execute_statement_list(inter, local_env, func->u.crowbar_f.block->statement_list);

    if (RETURN_STATEMENT_RESULT == result.type) {
        v = result.u.return_value;
    } else {
        v.type = CRB_NULL_VALUE;
    }

    push_value(inter, &v);
}

static void eval_function_call_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    FunctionDefinition *func;
    CRB_LocalEnvironment *local_env;
    char *identifier = expr->u.function_call_expression.identifier;

    func = crb_search_function(identifier);
    if (NULL == func) {
        crb_runtime_error(expr->line_number, FUNCTION_NOT_FOUND_ERR, STRING_MESSAGE_ARGUMENT, "name", identifier, MESSAGE_ARGUMENT_END);
    }
    
    local_env = alloc_local_environment(inter);
    switch (func->type) {
    case CROWBAR_FUNCTION_DEFINITION:
        call_crowbar_function(inter, local_env, env, expr, func);
        break;
    case NATIVE_FUNCTION_DEFINITION:
        call_native_function(inter, local_env, env, expr, func->u.native_f.proc);
        break;
    case FUNCTION_DEFINITION_TYPE_COUNT_PLUS_1:
    default:
        DBG_panic(("bad case..%d\n", func->type));
    }

    dispose_local_environment(inter);
}

static void eval_array_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionList *list)
{
    CRB_Value v;
    CRB_Value t;
    int size;
    ExpressionList *pos;
    int i;

    size = 0;
    for (pos = list; pos; pos = pos->next) {
        size++;
    }
    /* fprintf(stderr, "eval_array_expression size:%d\n", size); */

    v.type = CRB_ARRAY_VALUE;
    v.u.object = crb_create_array_i(inter, size);
    /* fprintf(stderr, "eval_array_expression size:%d create array ok\n", size); */
    push_value(inter, &v);
    /* fprintf(stderr, "eval_array_expression ^^^^^^ push size:%d push ok\n", size); */

    for (pos = list, i = 0; pos; pos = pos->next, ++i) {
        fprintf(stderr, "eval_array_expression eval_expression (env:%p,expr:%p)\n", env, pos->expression);
        eval_expression(inter, env, pos->expression);
        t = pop_value(inter);
        /* fprintf(stderr, "eval_array_expression $$$$$ pop t:%p size:%d push ok %p\n", &t, size, v.u.object->u.array.array); */
        v.u.object->u.array.array[i] = t;
    }
}

static void check_method_argument_count(int line_number, ArgumentList *arg_list, int arg_count) {
    ArgumentList *arg_p;
    int count = 0;

    for (arg_p = arg_list; arg_p; arg_p = arg_p->next) {
        ++count;
    }

    if (count < arg_count) {
        crb_runtime_error(line_number, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    } else if (count > arg_count) {
        crb_runtime_error(line_number, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
    }
}

static void eval_method_call_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *left;
    CRB_Value result;
    CRB_Boolean error_flag = CRB_FALSE;

    fprintf(stderr , "eval_method_call_expression eval_expression(env:%p expr:%p)\n", env, expr->u.method_call_expression.expression);
    eval_expression(inter, env, expr->u.method_call_expression.expression);
    left = peek_stack(inter, 0);

    /* 数组操作  */
    if (CRB_ARRAY_VALUE == left->type) {
        if (!strcmp(expr->u.method_call_expression.identifier, "add")) {
            CRB_Value *add;
            check_method_argument_count(expr->line_number, expr->u.method_call_expression.argument, 1);
            fprintf(stderr , "eval_method_call_expression eval_expression(env:%p expr:%p)\n", env, expr->u.method_call_expression.argument->expression);
            eval_expression(inter, env, expr->u.method_call_expression.argument->expression);
            add = peek_stack(inter, 0);
            crb_array_add(inter, left->u.object, *add);
            pop_value(inter);
            result.type = CRB_NULL_VALUE;

        } else if (!strcmp(expr->u.method_call_expression.identifier, "size")) {
            check_method_argument_count(expr->line_number, expr->u.method_call_expression.argument, 0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = left->u.object->u.array.size;

        } else if (!strcmp(expr->u.method_call_expression.identifier, "resize")) {
            CRB_Value new_size;
            check_method_argument_count(expr->line_number, expr->u.method_call_expression.argument, 1);
            fprintf(stderr, "eval_method_call_expression eval_expression(env:%p, expr:%p)\n", env, expr->u.method_call_expression.argument->expression);
            eval_expression(inter, env, expr->u.method_call_expression.argument->expression);
            new_size = pop_value(inter);
            if (new_size.type != CRB_INT_VALUE) {
                crb_runtime_error(expr->line_number, ARRAY_RESIZE_ARGUMENT_ERR, MESSAGE_ARGUMENT_END);
            }

            crb_array_resize(inter, left->u.object, new_size.u.int_value);
            result.type = CRB_NULL_VALUE;
        } else {
            error_flag = CRB_TRUE;
        }
    } else if (CRB_STRING_VALUE == left->type) {
        if (!strcmp(expr->u.method_call_expression.identifier, "length")) {
            check_method_argument_count(expr->line_number, expr->u.method_call_expression.argument, 0);
            result.type = CRB_INT_VALUE;
            result.u.int_value = strlen(left->u.object->u.string.string);
        } else {
            error_flag = CRB_TRUE;
        }
    }

    if (error_flag) {
        crb_runtime_error(expr->line_number, NO_SUCH_METHOD_ERR, STRING_MESSAGE_ARGUMENT, "method_name", expr->u.method_call_expression.identifier, MESSAGE_ARGUMENT_END);
    }

    pop_value(inter);
    push_value(inter, &result);
}

static void eval_index_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *left;
    left = get_array_element_lvalue(inter, env, expr);
    push_value(inter, left);
}

static void eval_inc_dec_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *operand;
    CRB_Value result;
    int old_value;

    operand = get_lvalue(inter, env, expr->u.inc_dec.operand);
    if (operand->type != CRB_INT_VALUE) {
        crb_runtime_error(expr->line_number, INC_DEC_OPERAND_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }

    old_value = operand->u.int_value;
    if (INCREMENT_EXPRESSION == expr->type) {
        operand->u.int_value++;
    } else {
        DBG_assert(DECREMENT_EXPRESSION == expr->type, ("expr->type:%d\n", expr->type));
        operand->u.int_value--;
    }
    
    result.type = CRB_INT_VALUE;
    result.u.int_value = old_value;
    push_value(inter, &result);
}

static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    /*fprintf(stderr, "eval_expression:%s ++++ line:%d expr:%p\n", getEvalType(expr->type), expr->line_number, expr);  */
    fprintf(stderr, "eval_expression %p\n", expr);
    switch (expr->type) {
        case BOOLEAN_EXPRESSION:
            eval_boolean_expression(inter, expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
            eval_int_expression(inter, expr->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            eval_double_expression(inter, expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            /* fprintf(stderr, "eval_expression %d %s--\n", expr->line_number, expr->u.string_value); */
            eval_string_expression(inter, expr->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            /* fprintf(stderr, "eval_expression IDENTIFIER_EXPRESSION %d %s\n", expr->type, expr->u.identifier); */
            eval_identifier_expression(inter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            fprintf(stderr, "eval_expression left:%p op:%p\n", expr->u.assign_expression.left, expr->u.assign_expression.operand); 
            eval_assign_expression(inter, env,
                    expr->u.assign_expression.left,
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
            eval_binary_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;

        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            eval_logical_and_or_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;

        case MINUS_EXPRESSION:
            /* fprintf(stderr, "eval_expression:%d MINUS_EXPRESSION ++++ line:%d\n", expr->type, expr->line_number); */
            crb_eval_minus_expression(inter, env, expr->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            /* fprintf(stderr, "eval_expression:%d FUNCTION_CALL_EXPRESSION ++++ line:%d\n", expr->type, expr->line_number); */
            eval_function_call_expression(inter, env, expr);
            break;
        case NULL_EXPRESSION:
            eval_null_expression(inter);
            break;
            /* v2 */
        case METHOD_CALL_EXPRESSION:
            /* fprintf(stderr, "eval_expression:%d METHOD_CALL_EXPRESSION ++++ line:%d\n", expr->type, expr->line_number); */
            eval_method_call_expression(inter, env, expr);
            break;
        case ARRAY_EXPRESSION:
            /* fprintf(stderr, "eval_expression:%d ARRAY_EXPRESSION ++++ line:%d\n", expr->type, expr->line_number); */
            eval_array_expression(inter, env, expr->u.array_literal);
            break;
        case INDEX_EXPRESSION:
            /* fprintf(stderr, "eval_expression INDEX_EXPRESSION:%d !!!!!!! line:%d\n", expr->type, expr->line_number); */
            eval_index_expression(inter, env, expr);
            break;
        case INCREMENT_EXPRESSION:
        case DECREMENT_EXPRESSION:
            eval_inc_dec_expression(inter, env, expr);
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad case. type:%d\n", expr->type));
    }
    /* fprintf(stderr, "eval_expression:%s ------------------ line:%d value:%s\n", getEvalType(expr->type), expr->line_number, CRB_value_to_string(peek_stack(inter, 0))); */
}


CRB_Value crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr)
{
    eval_expression(inter, env, expr);

    /* fprintf(stderr, "crb_eval_expression -- expr:%d\n", expr->type); */
    return pop_value(inter);
}

void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size)
{
    shrink_stack(inter, shrink_size);
}

/* vim: set tabstop=4 set shiftwidth=4 */

