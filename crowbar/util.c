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

    fprintf(stderr, "crb_search_global_variable identifier:%s variable:%p\n", identifier, inter->variable);
    for (pos = inter->variable; pos; pos = pos->next) {
        fprintf(stderr, "crb_search_global_variable identifier:%s pos:%p\n", identifier, pos);
        if (!strcmp(pos->name, identifier)) {
            return pos;
        }
    }

    return NULL;
}

Variable* crb_search_local_variable(CRB_LocalEnvironment *env, char *identifier)
{
    Variable *pos;
    fprintf(stderr, "crb_search_local_variable env:%p identifier:%s\n", env, identifier);
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

void* crb_execute_malloc(CRB_Interpreter *inter, size_t size)
{
    void *p;
    p = MEM_storage_malloc(inter->execute_storage, size);

    return p;
}

Variable* crb_add_global_variable(CRB_Interpreter *inter, char *identifier)
{
    Variable *new_variable;

    new_variable = crb_execute_malloc(inter, sizeof(Variable));
    new_variable->name = crb_execute_malloc(inter, strlen(identifier) + 1);
    strcpy(new_variable->name, identifier);
    new_variable->next = inter->variable;
    inter->variable = new_variable;

    return new_variable;
}

void* crb_malloc(size_t size) {
    void *p;
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();

    p = MEM_storage_malloc(inter->interpreter_storage, size);

    return p;
}

Variable* crb_add_local_variable(CRB_LocalEnvironment *env, char *identifier)
{
    Variable *new_variable;
    new_variable = MEM_malloc(sizeof(Variable));
    new_variable->name = identifier;
    new_variable->next = env->variable;
    env->variable = new_variable;

    return new_variable;
}

/* v2 */
void crb_vstr_clear(VString *str)
{
    if (NULL == str) {
        return;
    }
    str->string = NULL;
}

static int my_strlen(char *str)
{
    if (NULL == str) {
        return 0;
    }
    return strlen(str);
}

void crb_vstr_append_string(VString *v, char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = old_len + strlen(str) + 1;
    v->string = MEM_realloc(v->string, new_size);
    strcpy(&v->string[old_len], str);
}

char* CRB_value_to_string(CRB_Value *value)
{
    VString vstr;
    char buf[LINE_BUF_SIZE];
    int i;

    crb_vstr_clear(&vstr);

    switch (value->type) {
        case CRB_BOOLEAN_VALUE:
            if (value->u.boolean_value) {
                crb_vstr_append_string(&vstr, "true");
            } else {
                crb_vstr_append_string(&vstr, "false");
            }
            break;
        case CRB_INT_VALUE:
            sprintf(buf, "%d", value->u.int_value);
            crb_vstr_append_string(&vstr, buf);
            break;
        case CRB_DOUBLE_VALUE:
            sprintf(buf, "%f", value->u.double_value);
            crb_vstr_append_string(&vstr, buf);
            break;
        case CRB_STRING_VALUE:
            crb_vstr_append_string(&vstr, value->u.object->u.string.string);
            break;
        case CRB_NATIVE_POINTER_VALUE:
            sprintf(buf, "(%s:%p)",
                    value->u.native_pointer.info->name,
                    value->u.native_pointer.pointer);
            crb_vstr_append_string(&vstr, buf);
            break;
        case CRB_NULL_VALUE:
            crb_vstr_append_string(&vstr, "null");
            break;
        case CRB_ARRAY_VALUE:
            crb_vstr_append_string(&vstr, "(");
            for (i = 0; i < value->u.object->u.array.size; ++i) {
                char *new_str;
                if (i > 0) {
                    crb_vstr_append_string(&vstr, ", ");
                }
                new_str = CRB_value_to_string(&value->u.object->u.array.array[i]);
                crb_vstr_append_string(&vstr, new_str);
                MEM_free(new_str);
            }
            crb_vstr_append_string(&vstr, ")");
            break;
        default:
            DBG_panic(("value type:%d\n", value->type));
    }

    return vstr.string;
}


/* vim: set tabstop=4 set shiftwidth=4 */

