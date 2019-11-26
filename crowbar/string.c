/*
 * File : string.c
 * CreateDate : 2019-11-26 13:09:09
 * */

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "crowbar.h"

#define STRING_ALLOC_SIZE (256)

static char *st_string_literal_buffer = NULL;
static int st_string_literal_buffer_size = 0;
static int st_string_literal_buffer_alloc_size = 0;

void crb_open_string_literal(void)
{
    st_string_literal_buffer_size = 0;
}

void crB_add_string_literal(int letter)
{
    // 重新分配
    if (st_string_literal_buffer_size == st_string_literal_buffer_alloc_size) {
        st_string_literal_buffer_alloc_size += STRING_ALLOC_SIZE;
        st_string_literal_buffer = MEM_realloc(st_string_literal_buffer, st_string_literal_buffer_alloc_size);
    }

    st_string_literal_buffer[st_string_literal_buffer_size] = letter;
    st_string_literal_buffer_size++;
}


/* vim: set tabstop=4 set shiftwidth=4 */

