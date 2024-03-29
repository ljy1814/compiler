/*
 * File : storage.c
 * CreateDate : 2019-11-25 12:03:17
 * */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "memory.h"

/* 打开一个存储器 */
MEM_Storage MEM_open_storage_func(MEM_Controller controller, char *filename, int line, int page_size)
{
    MEM_Storage storage;

    storage = MEM_malloc_func(controller, filename, line, sizeof(struct MEM_Storage_tag));
    storage->page_list = NULL;
    assert(page_size >= 0);

    if (page_size > 0) {
        storage->current_page_size = page_size;
    } else {
        storage->current_page_size = DEFAULT_PAGE_SIZE;
    }

    return storage;
}

/* 分配存储器 */
void* MEM_storage_malloc_func(MEM_Controller controller, char *fn, int line, MEM_Storage storage, size_t size)
{
    int cell_num;
    MemoryPage *new_page;
    void *p;

    cell_num = ((size - 1) / CELL_SIZE) + 1;
    if (storage->page_list != NULL && 
            (storage->page_list->use_cell_num + cell_num < storage->page_list->cell_num)) { /* enough */
        p = &(storage->page_list->cell[storage->page_list->use_cell_num]);
        storage->page_list->use_cell_num += cell_num;
    } else {
        int alloc_cell_num;
        alloc_cell_num = larger(cell_num, storage->current_page_size);
        new_page = MEM_malloc_func(controller, fn, line, sizeof(MemoryPage) + CELL_SIZE * (alloc_cell_num - 1) );
        new_page->next = storage->page_list; /* 接到前面 */
        new_page->cell_num = alloc_cell_num;
        storage->page_list = new_page;

        p = &(new_page->cell[0]);
        new_page->use_cell_num = cell_num;
    }

    return p;
}


void MEM_dispose_storage_func(MEM_Controller controller, MEM_Storage storage)
{
    MemoryPage *tmp;

    fprintf(stderr, "MEM_dispose_storage_func -----\n");
    while(storage->page_list) {
        tmp = storage->page_list->next;
        MEM_free_func(controller, storage->page_list);
        storage->page_list = tmp;
    }
    MEM_free_func(controller, storage);
}

/* vim: set tabstop=4 set shiftwidth=4 */

