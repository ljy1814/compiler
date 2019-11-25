/*
 * File : interpreter.c
 * CreateDate : 2019-11-25 03:41:24
 * */

#include "MEM.h"
#include "DBG.h"
#define GLOBAL_VARIABLE_DEFINE
#include "crowbar.h"

CRB_Interpreter *CRB_create_interpreter(void)
{
    MEM_Storage storage;
    CRB_Interpreter *interpreter;

    // 打开一个存储器
    storage = MEM_open_storage(0);
    // 创建一个解释器
    interpreter = MEM_storage_malloc(storage, sizeof(struct CRB_Interpreter_tag));
    interpreter->interpreter_storage = storage; //
    interpreter->execute_storage = NULL; //
    interpreter->variable = NULL;
    interpreter->function_list = NULL;
    interpreter->statement_list = NULL;
    interpreter->current_line_number = 1;

    crb_set_current_interpreter(interpreter);
    add_native_functions(interpreter);  // 注册内置函数

    return interpreter;
}

void CRB_compile(CRB_Interpreter *interpreter, FILE *fp)
{
    extern int yyparse(void);
    extern FILE *yyin;

    crb_set_current_interpreter(interpreter);

    yyin = fp;
    if (yyparse()) {
        fprintf(stderr, "Error\n");
        exit(1);
    }

    crb_reset_string_literal_buffer(); // 重置字符串缓存
}

void CRB_interpreter(CRB_Interpreter *interpreter)
{
    interpreter->execute_storage = MEM_open_storage(0);
    crb_add_std_fp(interpreter);
    crb_execute_statement_list(interpreter, NULL, interpreter->statement_list);
}

void CRB_dispose_interpreter(CRB_Interpreter *interpreter)
{
    release_global_strings(interpreter);

    if (interpreter->execute_storage) {
        MEM_dispose_storage(interpreter->execute_storage);
    }
    MEM_dispose_storage(interpreter->interpreter_storage);
}


/* vim: set tabstop=4 set shiftwidth=4 */
