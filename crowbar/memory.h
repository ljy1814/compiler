#ifndef PRIVATE_MEM_H_INCLUDED
#define PRIVATE_MEM_H_INCLUDED

#include "MEM.h"

#define DEFAULT_PAGE_SIZE (1024)

typedef union Header_tag Header;

struct MEM_Controller_tag {
    FILE *error_fp;
    MEM_ErrorHandler error_handler;
    MEM_FailMode fail_mode;
    Header *block_header;
};

// 整数  小数 或者 指针
typedef union {
    long l_dummy;
    double d_dummy;
    void *p_dummy;
} Cell;

#define CELL_SIZE (sizeof(Cell))

typedef struct MemoryPage_tag MemoryPage;
typedef MemoryPage *MemoryPageList;

struct MemoryPage_tag {
    int cell_num;
    int use_cell_num;
    MemoryPageList next;
    Cell cell[1];
};

typedef struct MEM_Storage_tag *MEM_Storage;

struct MEM_Storage_tag {
    MemoryPageList page_list;
    int current_page_size; // 当前页
};



typedef union {
    long l_dummy;
    double d_dummy;
    void *p_dummy;
} Align;

#define MARK (0xCD)
#define MARK_SIZE (4)
#define ALIGN_SIZE (sizeof(Align))
#define revalue_up_align(val) ((val) ? (((val) - 1) / ALIGN_SIZE + 1) : 0 )
#define HEADER_ALIAGN_SIZE (revalue_up_align(sizeof(HeaderStruct)))

#define larger(a , b) ( ((a) > (b)) ? (a) : (b) )

typedef struct {
    int size;
    char *filename;
    int line;
    Header *prev;
    Header *next;
    unsigned char mark[MARK_SIZE];
} HeaderStruct;

union Header_tag {
    HeaderStruct s;
    Align u[HEADER_ALIAGN_SIZE];
};

#endif
