/* 面向普通用户 */
#ifndef PUBLIC_CRB_H_INCLUDED
#define PUBLIC_CRB_H_INCLUDED

#include <stdio.h>

typedef struct CRB_Interpreter_tag CRB_Interpreter; /* 指向解释器的指针 */

CRB_Interpreter *CRB_create_interpreter(void);

void CRB_compile(CRB_Interpreter *interpreter, FILE *fp);  /* 生成分析树 */

void CRB_interpret(CRB_Interpreter *interpreter);  /* 运行 */

void CRB_dispose_interpreter(CRB_Interpreter *interpreter);  /* 执行完之后回收解释器 */

#endif
