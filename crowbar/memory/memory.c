/*
 * File : memory.c
 * CreateDate : 2019-11-25 11:55:41
 * */
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void default_error_handler(MEM_Controller controller, char *filename, int line, char *msg);

static struct MEM_Controller_tag st_default_controller = {
    NULL, 
    default_error_handler,
    MEM_FAIL_AND_EXIT
};

MEM_Controller mem_default_controller = &st_default_controller;

#ifdef DEBUG

void check_mark_sub(unsigned char *mark, int size)
{
    int i;
    /* 
    fprintf(stderr, "mark %p:%d {", mark, size);
    for (i = 0; i < size; ++i) {
        fprintf(stderr, "%x,", mark, size, (int)mark[i]);
    }
    fprintf(stderr, "}\n");
    */
    for (i = 0; i < size; ++i) {
        if (mark[i] != MARK) {
            fprintf(stderr, "bad mark %p:%d {%x,%x}\n", mark, size, mark[0], mark[1]);
            abort();
        }
    }
}

/*
 *      | size       |
 *      | filename   |
 *      | line       |
 * <-   | <- prev    | 
 *      | next ->    | ->
 *      | [4] mark   |    <--------
 *      | union data |
 *      |  data      |
 *      | CD CD CD CD| <- tail
 *      ~~~~~~~~~~~~~~  <--------header[1]
 *      ~~~~~~~~~~~~~~
 * */

void check_mark(Header *header)
{
    unsigned char *tail;
    tail = ((unsigned char *)header) + header->s.size + sizeof(Header);
    fprintf(stderr,  "Header[size:%d,filename:%s,line:%d prev:%p next:%p mark:%p {%x,%x,%x,%x} tail:%p {%x,%x,%x,%x}]\n", header->s.size, header->s.filename, header->s.line, header->s.prev, header->s.next,header->s.mark, header->s.mark[0], header->s.mark[1], header->s.mark[2], header->s.mark[3], tail, tail[0],tail[1], tail[2], tail[2]);  
    check_mark_sub(header->s.mark, (char *)&header[1] - (char *)header->s.mark);
    check_mark_sub(tail, MARK_SIZE);
}

static void rechain_block(MEM_Controller controller, Header *header)
{
    if (header->s.prev) {
        header->s.prev->s.next = header;
    } else {
        controller->block_header = header;
    }

    if (header->s.next) {
        header->s.next->s.prev = header;
    }
}

static void unchain_block(MEM_Controller controller, Header *header)
{
    if (header->s.prev) {
        header->s.prev->s.next = header->s.next;
    } else {
        controller->block_header = header->s.next;
    }

    if (header->s.next) {
        header->s.next->s.prev = header->s.prev;
    }
}

#endif


void MEM_dump_blocks_func(MEM_Controller controller, FILE *fp)
{
#ifdef DEBUG
    Header *pos;
    int counter = 0;

    for (pos = controller->block_header; pos; pos = pos->s.next) {
        check_mark(pos);
        fprintf(fp, "[%04d]%p***************\n", counter, (char*)pos + sizeof(Header));
        fprintf(fp, "%s line %d size...%d\n", pos->s.filename, pos->s.line, pos->s.size);
        counter++;
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
    header->s.filename = filename;
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
    /* header data tail */
    alloc_size = size + sizeof(Header) + MARK_SIZE;
#else
    alloc_size = size ;
#endif

    ptr = malloc(alloc_size);
    if (NULL == ptr) {
        error_handler(controller, filename, line, "malloc");
    }

#ifdef DEBUG
    memset(ptr, NULL_VALUE, alloc_size);
    set_header(ptr, size, filename, line);
    set_tail(ptr, alloc_size);
    chain_block(controller, (Header*)ptr);
    ptr = (char*)ptr + sizeof(Header);
#endif

    return ptr;
}

void MEM_free_func(MEM_Controller controller, void *ptr)
{
    void *real_ptr;
    Header *header;
    unsigned char *tail;
#ifdef DEBUG
    int size;
#endif
    if (NULL == ptr) {
        return ;
    }

#ifdef DEBUG
    real_ptr = (char *)ptr - sizeof(Header);
    header = (Header*) real_ptr;
    tail = ((unsigned char *)header) + header->s.size + sizeof(Header);
    fprintf(stderr, "MEM_free_func pre:%p real_ptr:%p\n", ptr, real_ptr);
    fprintf(stderr,  "Header[size:%d,filename:%s,line:%d prev:%p next:%p mark:%p {%x,%x,%x,%x} tail:%p {%x,%x,%x,%x}]\n", header->s.size, header->s.filename, header->s.line, header->s.prev, header->s.next,header->s.mark, header->s.mark[0], header->s.mark[1], header->s.mark[2], header->s.mark[3], tail, tail[0],tail[1], tail[2], tail[2]);  
    check_mark((Header*)real_ptr);
    size = ((Header*)real_ptr)->s.size;
    unchain_block(controller, real_ptr);
    memset(real_ptr, NULL_VALUE, size + sizeof(Header));
#else
    real_ptr = ptr;
#endif

    free(real_ptr);
}

void* MEM_realloc_func(MEM_Controller controller, char *fn, int line, void *ptr, size_t size)
{
    void *new_ptr;
    size_t alloc_size;
    void *real_ptr;

    fprintf(stderr, "MEM_realloc_func fn:%s line:%d ptr:%p size:%d\n", fn, line, ptr, size);
#ifdef DEBUG
    Header old_header;
    int old_size;
    alloc_size = size + sizeof(Header) + MARK_SIZE;

    if (NULL != ptr) {
        real_ptr = (char *)ptr - sizeof(Header);
        check_mark((Header*)real_ptr);
        old_header = *((Header*)real_ptr);
        old_size = old_header.s.size;
        unchain_block(controller, real_ptr);
    } else {
        real_ptr = NULL;
        old_size = 0;
    }
#else
    alloc_size = size;
    real_ptr = ptr;
#endif

    new_ptr = realloc(real_ptr, alloc_size);
    if (NULL == new_ptr) {
        if (NULL == ptr) {
            error_handler(controller, fn, line, "realloc(malloc)");
        } else {
            error_handler(controller, fn, line, "realloc");
            free(real_ptr);
        }
    }

#ifdef DEBUG
    if (ptr) {
        *((Header*)new_ptr) = old_header;
        ((Header*)new_ptr)->s.size = size;
        rechain_block(controller, (Header*)new_ptr);
        set_tail(new_ptr, alloc_size);
    } else {
        set_header(new_ptr, size, fn, line);
        set_tail(new_ptr, alloc_size);
        chain_block(controller, (Header*)new_ptr);
    }
    new_ptr = (char *)new_ptr + sizeof(Header);
    if (size > old_size) {
        memset((char *)new_ptr + old_size, NULL_VALUE, size - old_size);
    }
#endif

    return new_ptr;
}

char* MEM_strdup_func(MEM_Controller controller, char *fn, int line, char *str)
{
    char *ptr;
    int size;
    size_t alloc_size;

    size = strlen(str) + 1;

#ifdef DEBUG
    alloc_size = size + sizeof(Header) + MARK_SIZE;
#else
    alloc_size = size;
#endif

    ptr = malloc(alloc_size);
    if (NULL == ptr) {
        error_handler(controller, fn, line, "strdup");
    }

#ifdef DEBUG
    memset(ptr, NULL_VALUE, alloc_size);
    set_header((Header*)ptr, size, fn, line);
    set_tail(ptr, alloc_size);
    chain_block(controller, (Header*)ptr);
    ptr = (char*)ptr + sizeof(Header);
#endif

    strcpy(ptr, str);

    return ptr;
}

/* vim: set tabstop=4 set shiftwidth=4 */

