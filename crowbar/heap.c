/*
 * File : heap.c
 * CreateDate : 2019-12-03 08:43:04
 * */
#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

/*
 * mark对象,数组则mark其每个元素
 * */
static void gc_mark(CRB_Object *obj)
{
    int i;
    if (obj->marked) {
        return;
    }

    obj->marked = CRB_TRUE;

    if (ARRAY_OBJECT != obj->type) {
        return;
    }

    for (i = 0; i < obj->u.array.size; ++i) {
        if (dkc_is_object_value(obj->u.array.array[i].type)) {
            gc_mark(obj->u.array.array[i].u.object);
        }
    }
}

static void gc_reset_mark(CRB_Object *obj)
{
    obj->marked = CRB_FALSE;
}

static void gc_mark_ref_in_native_method(CRB_LocalEnvironment *env)
{
    RefInNativeFunc *ref;

    for (ref = env->ref_in_native_method; ref; ref = ref->next) {
        gc_mark(ref->object);
    }
}

static void gc_mark_objects(CRB_Interpreter *inter)
{
    CRB_Object *obj;
    Variable *v;
    CRB_LocalEnvironment *env;
    int i;

    /*
     * 堆上对象全部取消mark
     * */
    for (obj = inter->heap.header; obj; obj = obj->next) {
        gc_reset_mark(obj);
    }

    /*
     * 全局变量全部标记
     * */
    for (v = inter->variable; v; v = v->next) {
        if (dkc_is_object_value(v->value.type)) {
            gc_mark(v->value.u.object);
        }
    }

    /*
     * 局部环境,引用的对象全部mark
     * 内置函数引用的对象全部mark
     * */
    for (env = inter->top_environment; env; env = env->next) {
        for (v = env->variable; v; v = v->next) {
            if (dkc_is_object_value(v->value.type)) {
                gc_mark(v->value.u.object);
            }
        }
        gc_mark_ref_in_native_method(env);
    }

    /*
     * 栈上的对象也全部标记
     * */
    for (i = 0; i < inter->stack.stack_pointer; ++i) {
        if (dkc_is_object_value(inter->stack.stack[i].type)) {
            gc_mark(inter->stack.stack[i].u.object);
        }
    }
}

static void gc_dispose_object(CRB_Interpreter *inter, CRB_Object *obj)
{
    switch (obj->type) {
        case ARRAY_OBJECT:
            inter->heap.current_heap_size -= sizeof(CRB_Value) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.array);
            break;
        case STRING_OBJECT:
            inter->heap.current_heap_size -= strlen(obj->u.string.string) + 1;
            MEM_free(obj->u.string.string);
            break;
        case OBJECT_TYPE_COUNT_PLUS_1:
        default:
            DBG_assert(0, ("bad type:%d\n", obj->type));
    }

    inter->heap.current_heap_size -= sizeof(CRB_Object);
    MEM_free(obj);
}

static void gc_sweep_objects(CRB_Interpreter *inter)
{
    CRB_Object *obj;
    CRB_Object *tmp;

    for (obj = inter->heap.header; obj;) {
        if (!obj->marked) {
            if (obj->prev) {
                obj->prev->next = obj->next;
            } else {
                inter->heap.header = obj->next;
            }

            if (obj->next) {
                obj->next->prev = obj->prev;
            }
            tmp = obj->next;
            gc_dispose_object(inter, obj);
            obj = tmp;
        } else {
            obj = obj->next;
        }
    }
}

void crb_garbage_collect(CRB_Interpreter *inter)
{
    gc_mark_objects(inter);
    gc_sweep_objects(inter);
}

static void check_gc(CRB_Interpreter *inter)
{
#if 0
    crb_garbage_collect(inter);
#endif

    if (inter->heap.current_heap_size > inter->heap.current_threshold) {
        crb_garbage_collect(inter);
        inter->heap.current_threshold = inter->heap.current_heap_size + HEAP_THRESHOLD_SIZE;
    }
}

static CRB_Object* alloc_object(CRB_Interpreter *inter, ObjectType type)
{
    CRB_Object *ret;
    check_gc(inter);
    ret = MEM_malloc(sizeof(CRB_Object));
    inter->heap.current_heap_size += sizeof(CRB_Object);
    ret->type = type;
    ret->marked = CRB_FALSE;
    ret->prev = NULL;
    ret->next = inter->heap.header;
    inter->heap.header = ret;

    if (ret->next) {
        ret->next->prev = ret;
    }

    return ret;
}

CRB_Object* crb_create_array_i(CRB_Interpreter *inter, int size)
{
    CRB_Object *ret;

    ret = alloc_object(inter, ARRAY_OBJECT);
    ret->u.array.size = size;
    ret->u.array.alloc_size = size;
    ret->u.array.array = MEM_malloc(sizeof(CRB_Value) * size);
    inter->heap.current_heap_size += sizeof(CRB_Value) * size;
    return ret;
}

static void add_ref_in_native_method(CRB_LocalEnvironment *env, CRB_Object *obj)
{
    RefInNativeFunc *new_ref;

    new_ref = MEM_malloc(sizeof(RefInNativeFunc));
    new_ref->object = obj;
    new_ref->next = env->ref_in_native_method;
    env->ref_in_native_method = new_ref;
}

CRB_Object* CRB_create_array(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int size)
{
    CRB_Object *ret;
    ret = crb_create_array_i(inter, size);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Object* crb_create_crowbar_string_i(CRB_Interpreter *inter, char *str)
{
    CRB_Object *obj;
    obj = alloc_object(inter, STRING_OBJECT);
    obj->u.string.string = str;
    inter->heap.current_heap_size += strlen(str) + 1;
    obj->u.string.is_literal = CRB_FALSE;

    return obj;
}

CRB_Object* crb_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, char *str)
{
    CRB_Object *obj;
    obj = crb_create_crowbar_string_i(inter, str);
    add_ref_in_native_method(env, obj);

    return obj;
}

CRB_Object* crb_literal_to_crb_string(CRB_Interpreter *inter, char *str)
{
    CRB_Object *ret;
    ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    /* inter->heap.current_heap_size += strlen(str) + 1; */
    ret->u.string.is_literal = CRB_FALSE;

    return ret;
}

void crb_array_add(CRB_Interpreter *inter, CRB_Object *obj, CRB_Value v)
{
    int new_size;
    DBG_assert(ARRAY_OBJECT == obj->type, ("bad type:%d\n", obj->type));

    check_gc(inter);
    if (obj->u.array.size + 1 > obj->u.array.alloc_size) {
        new_size = obj->u.array.alloc_size * 2;
        if (0 == new_size || new_size - obj->u.array.alloc_size > ARRAY_ALLOC_SIZE) {
            new_size = obj->u.array.alloc_size + ARRAY_ALLOC_SIZE;
        }

        obj->u.array.array = MEM_realloc(obj->u.array.array, new_size * sizeof(CRB_Value));
        inter->heap.current_heap_size += (new_size - obj->u.array.alloc_size) * sizeof(CRB_Value);
        obj->u.array.alloc_size = new_size;
    }
    obj->u.array.array[obj->u.array.size] = v;
    obj->u.array.size++;
}

void crb_array_resize(CRB_Interpreter *inter, CRB_Object *obj, int new_size)
{
    int new_alloc_size;
    CRB_Boolean need_realloc;
    int i;

    check_gc(inter);

    if (new_size > obj->u.array.alloc_size) {
        new_alloc_size = obj->u.array.alloc_size * 2;
        if (new_alloc_size < new_size) {
            new_alloc_size = new_size + ARRAY_ALLOC_SIZE;
        } else if (new_alloc_size - obj->u.array.alloc_size > ARRAY_ALLOC_SIZE){
            new_alloc_size = obj->u.array.alloc_size + ARRAY_ALLOC_SIZE;
        }
        need_realloc = CRB_TRUE;
    } else if (obj->u.array.alloc_size - new_size > ARRAY_ALLOC_SIZE) {
        /* 减少数组长度 */
        new_alloc_size = new_size;
        need_realloc = CRB_TRUE;
    } else {
        need_realloc = CRB_FALSE;
    }

    if (need_realloc) {
        check_gc(inter);
        obj->u.array.array = MEM_realloc(obj->u.array.array, new_alloc_size * sizeof(CRB_Value));
        inter->heap.current_heap_size += (new_alloc_size - obj->u.array.alloc_size) * sizeof(CRB_Value);
        obj->u.array.alloc_size = new_alloc_size;
    }

    for (i = obj->u.array.size; i < new_size; ++i) {
        obj->u.array.array[i].type = CRB_NULL_VALUE;
    }
    obj->u.array.size = new_size;
}

/* vim: set tabstop=4 set shiftwidth=4 */

