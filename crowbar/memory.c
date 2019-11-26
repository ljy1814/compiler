/*
 * File : memory.c
 * CreateDate : 2019-11-25 11:55:41
 * */
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void MEM_dump_blocks_func(MEM_Controller controller, FILE *fp)
{
#ifdef DEBUG
    Header *pos;
    int counter = 0;

    for (pos = controller->block_header; pos; pos = pos->s.next) {
        //check_mark(pos);
    }

#endif
}

static void default_error_handler(MEM_Controller controller, char *filename, int line, char *msg)
{
    fprintf(controller->error_fp, "MEM:%s failed in %s at %d\n", msg, filename, line);
}

static void error_handler(MEM_Controller controller, char *filename, int line, char *msg)
{
    if (NULL == controller->error_fp) {
        controller->error_fp = stderr;
    }

    controller->error_handler(controller, filename, line, msg);

    if (MEM_FAIL_AND_EXIT == controller->fail_mode) {
        exit(1);
    }
}

void set_header(Header *header, int size, char *filename, int line)
{
    header->s.size = size;
    header->s.filename = size;
    header->s.line = size;
    memset(header->s.mark, MARK, (char*)&header[1] -(char*)header->s.mark);
}

void set_tail(void *ptr, int alloc_size)
{
    char *tail;
    tail = ((char *)ptr) + alloc_size - MARK_SIZE;
    memset(tail, MARK, MARK_SIZE);
}

#ifdef DEBUG 

static void chain_block(MEM_Controller controller, Header *new_header)
{
    if (controller->block_header) {
        controller->block_header->s.prev = new_header;
    }
    new_header->s.prev = NULL;
    new_header->s.next = controller->block_header;
    controller->block_header = new_header;
}

#endif

void *MEM_malloc_func(MEM_Controller controller, char *filename, int line, size_t size)
{
    void *ptr;
    size_t alloc_size;

#ifdef DEBUG
    alloc_size = size + sizeof(Header) + MARK_SIZE;
#else
    alloc_size = size ;
#endif

    ptr = malloc(alloc_size);
    if (NULL == ptr) {
        error_handler(controller, filename, line, "malloc");
    }

#ifdef DEBUG
    memset(pqtr, 0xcc, alloc_size);
    set_header(ptr, size, filename, line);
    set_tail(ptr, alloc_size);
    chain_block(controller, (Header*)ptr);
    ptr = (char*)ptr + sizeof(Header);
#endif

    return ptr;
}


/* vim: set tabstop=4 set shiftwidth=4 */
