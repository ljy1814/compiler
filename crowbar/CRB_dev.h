/* 面向开发者 */
#ifndef PUBLIC_CRB_DEV_H_INCLUDED
#define PUBLIC_CRB_DEV_H_INCLUDED

#include "CRB.h"

/* bool类型 */
typedef enum {
    CRB_FALSE = 0,
    CRB_TRUE = 1
} CRB_Boolean;

/* 字符串类型 */
typedef struct CRB_String_tag CRB_String;
/* v2 local env */
typedef struct CRB_LocalEnvironment_tag CRB_LocalEnvironment;
typedef struct CRB_Object_tag CRB_Object;
typedef struct CRB_Array_tag CRB_Array;

/* v2 end */

/* 指针类型 */
typedef struct {
    char *name;
} CRB_NativePointerInfo;

/* 值 类型 */
typedef enum {
    CRB_BOOLEAN_VALUE = 1,
    CRB_INT_VALUE ,
    CRB_DOUBLE_VALUE ,
    CRB_STRING_VALUE ,
    CRB_NATIVE_POINTER_VALUE ,
    CRB_NULL_VALUE ,
    CRB_ARRAY_VALUE
} CRB_ValueType;

/* 指针类型 */
typedef struct {
    CRB_NativePointerInfo *info;
    void *pointer;
} CRB_NativePointer;


typedef struct {
    CRB_ValueType type;
    union {
        CRB_Boolean boolean_value;
        int int_value;
        double double_value;
        /* CRB_String *string_value; */
        CRB_NativePointer native_pointer;
        CRB_Object *object;
    } u;
} CRB_Value;

/* 调用c语言函数 */
typedef CRB_Value CRB_NativeFunctionProc(CRB_Interpreter *interpreter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args);

/* 注册c语言函数 */
void CRB_add_native_function(CRB_Interpreter *interpreter, char *name, CRB_NativeFunctionProc *proc);

/* 注册全局函数 */
void CRB_add_global_variable(CRB_Interpreter *inter, char *identifier, CRB_Value *value);

/* v2 */
CRB_Object* CRB_create_array(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int size);
char* CRB_value_to_string(CRB_Value *value);
CRB_Object* crb_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, char *str);
void chain_string(CRB_Interpreter *inter, CRB_Value *left, CRB_Value *right, CRB_Value *result);

#endif
