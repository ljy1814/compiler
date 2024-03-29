#ifndef PUBLIC_DBG_H_INCLUDED
#define PUBLIC_DBG_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>

/* 调试 */

typedef struct DBG_Controller_tag *DBG_Controller;
void DBG_set(DBG_Controller controller, char *file, int line);
void DBG_set_expression(char *expr);


#ifdef DBG_NO_DEBUG
    #define DBG_create_controller() ((void)0)
    #define DBG_set_debug_level(level) ((void)0)
    #define DBG_set_debug_write_fp(fp) ((void)0)
    #define DBG_assert(expr, arg) ((void)0)
    #define DBG_panic(arg) ((void)0)
    #define DBG_debug_write(arg) ((void)0)
#else
    #ifdef DBG_CONTROLLER
        #define DBG_CURRENT_CONTROLLER DBG_CONTROLLER
    #else
        #define DBG_CURRENT_CONTROLLER dbg_default_controller
    #endif
    
    extern DBG_Controller DBG_CURRENT_CONTROLLER;

    #define DBG_create_controller() (DBG_create_controller_func())
    #define DBG_set_debug_level(level) (DBG_set_debug_level_func(DBG_CURRENT_CONTROLLER, (level)))
    #define DBG_set_debug_write_fp(fp) (DBG_set_debug_write_fp(DBG_CURRENT_CONTROLLER, (fp)))
    #define DBG_assert(expr, arg) ((expr) ? (void)(0) : ((DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)), (DBG_set_expression(#expr)), DBG_assert_func arg))
    #define DBG_panic(arg) ( (DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)), DBG_panic_func arg)
    #define DBG_debug_write(arg) ( (DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)), DBG_debug_write_func arg )

#endif

typedef enum {
    DBG_TRUE = 1,
    DBG_FALSE = 0
} DBG_Boolean;

DBG_Controller DBG_create_controller_func(void);
void DBG_set_debug_level_func(DBG_Controller controller, int level);
void DBG_set_debug_write_fp_func(DBG_Controller controller, FILE *fp);
void DBG_assert_func(char *fmt, ...);
void DBG_panic_func(char *fmt, ...);
void DBG_debug_write_func(int level, char *fmt, ...);

#endif
