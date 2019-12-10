/*
 * File : error_message.c
 * CreateDate : 2019-11-24 01:59:28
 * */


#include <string.h>
#include "crowbar.h"

MessageFormat crb_compile_error_message_format[] = {
    {
        "dummy",
    },
    {
        "在($(token))附近发生语法错误",
    },
    {
        "不正确的字符($(bad_char))",
    },
    {
        "函数名重复($(name))",
    },
    {
        "dummy",
    }
};

MessageFormat crb_runtime_error_message_format[] = {
    {
        "dummy",
    },
    {
        "找不到变量($(name)).",
    },
    {
        "找不到函数($(name)).",
    },
    {
        "实际传入的参数比函数需要的参数多",
    },
    {
        "实际传入的参数比函数需要的参数少",
    },
    {
        "条件表达式的值必须是boolean类型",
    },
    {
        "减法运算符的操作数必须是政治型",
    },
    {
        "双目运算符$(operator)的操作数不正确",
    },
    {
        "$(operator)运算符不能用于boolean类型",
    },
    {
        "fopen参数不正确",
    },
    {
        "fclose参数不正确",
    },
    {
        "fgets参数不正确",
    },
    {
        "fputs参数不正确",
    },
    {
        "null只能用于== 和 != .(不能进行$(operator)操作).",
    },
    {
        "不能被0除",
    },
    {
        "全局变量$(name)不存在",
    },
    {
        "不能在函数外使用global语句",
    },
    {
        "运算符$(operator)不能用于字符串",
    },
    /* v2 */
    {
        "操作数不是左值",
    },
    {
        "下标运算符的左边操作数不是数组类型",
    },
    {
        "下标运算符中的值不是整数类型",
    },
    {
        "数组下标越界,数组大小:$(size),下标:[$(index)]",
    },
    {
        "对象中不存在名为$(method_name)的成员",
    },
    {
        "请为new_array()函数传入整数类型(数组大小).",
    },
    {
        "自增/自减目标值不是整数类型",
    },
    {
        "数组的resize()必须传入整数类型",
    },
    {
        "dummy",
    }
};

/* vim: set tabstop=4 set shiftwidth=4 */

