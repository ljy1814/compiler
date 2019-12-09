/*
 * File : stack.c
 * CreateDate : 2019-12-04 12:46:34
 * */

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

void push_value(CRB_Interpreter *inter, CRB_Value *value)
{
    DBG_assert(inter->stack.stack_pointer <= inter->stack.stack_alloc_size, ("stack_pointer:%d alloc_size:%d\n", inter->stack.stack_pointer, inter->stack.stack_alloc_size));

    if (inter->stack.stack_pointer == inter->stack.stack_alloc_size) {
        inter->stack.stack_alloc_size += STACK_ALLOC_SIZE;
        inter->stack.stack = MEM_realloc(inter->stack.stack, sizeof(CRB_Value) * inter->stack.stack_alloc_size );
    }

    inter->stack.stack[inter->stack.stack_pointer] = *value;
    inter->stack.stack_pointer++;
    fprintf(stderr, "push_value pointer:%d %s ok\n", inter->stack.stack_pointer, CRB_value_to_string(value));
}

CRB_Value pop_value(CRB_Interpreter *inter)
{
    CRB_Value ret;
    ret = inter->stack.stack[inter->stack.stack_pointer-1];
    inter->stack.stack_pointer--;
    fprintf(stderr, "pop_value :%d pointer:%d \n", ret.type, inter->stack.stack_pointer);
    fprintf(stderr, "pop_value %s ok\n", CRB_value_to_string(&ret));
    return ret;
}

CRB_Value* peek_stack(CRB_Interpreter *inter, int index)
{
    CRB_Value *ret;
    ret = &inter->stack.stack[inter->stack.stack_pointer - index - 1];
    fprintf(stderr, "peek_value %s ok\n", CRB_value_to_string(ret));
    return ret;
}

void shrink_stack(CRB_Interpreter *inter, int shrink_size)
{
    inter->stack.stack_pointer -= shrink_size;
}


/* vim: set tabstop=4 set shiftwidth=4 */

