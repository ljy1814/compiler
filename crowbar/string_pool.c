/*
 * File : string_pool.c
 * CreateDate : 2019-11-27 07:38:23
 * */

#include <stdio.h>
#include <stdlib.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static CRB_String *alloc_crb_string(CRB_Interpreter *inter, char *str, CRB_Boolean is_literal)
{
    CRB_String *ret;

    ret = MEM_malloc(sizeof(CRB_String));
    /* 引用计数 */
    ret->ref_count = 0;
    ret->is_literal = is_literal;
    ret->string = str;

    return ret;
}

CRB_String* crb_literal_to_crb_string(CRB_Interpreter *inter, char *str)
{
    CRB_String *ret;
    ret = alloc_crb_string(inter, str, CRB_TRUE);
    ret->ref_count = 1;

    return ret;
}

void crb_refer_string(CRB_String *str)
{
    str->ref_count++;
}

void crb_release_string(CRB_String *str)
{
    str->ref_count--;
    DBG_assert(str->ref_count >= 0, ("str->ref_count..%d\n", str->ref_count));

    if (0 == str->ref_count) {
        if (!str->is_literal) {
            MEM_free(str->string);
        }
        MEM_free(str);
    }
}

/*  v1
CRB_String* crb_create_crowbar_string(CRB_Interpreter *inter, char *str)
{
    CRB_String *ret = alloc_crb_string(inter, str, CRB_FALSE);
    ret->ref_count = 1;

    return ret;
}
*/

/* vim: set tabstop=4 set shiftwidth=4 */

