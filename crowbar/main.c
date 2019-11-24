/*
 * File : main.c
 * CreateDate : 2019-11-25 03:32:24
 * */

#include <stdio.h>
#include "CRB.h"
#include "MEM.h"

int main(int argc , char* argv[])
{
    CRB_Interpreter *interpreter;
    FILE *fp;

    if (argc != 2) {
        fprintf(stderr, "usage:%s filename", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (NULL == fp) {
        fprintf(stderr, "%s not found", argv[1]);
        exit(1);
    }


    /* 创建解释器 */
    interpreter = CRB_create_interpreter();
    /* 编译 */
    CRB_compile(interpreter, fp);
    /* 解释 */
    CRB_interpreter(interpreter);
    /* 释放解释器 */
    CRB_dispose_interpreter(interpreter);

    MEM_dump_blocks(stdout);

    return 0;
}
/* vim: set tabstop=4 set shiftwidth=4 */

