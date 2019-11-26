/*
 * File : util.c
 * CreateDate : 2019-11-26 07:40:14
 * */

#include <stdio.h>
#include <stdlib.h>
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

/* vim: set tabstop=4 set shiftwidth=4 */

