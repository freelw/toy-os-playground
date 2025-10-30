#ifndef MM_H
#define MM_H

#define PAGE_SIZE 4096
#define PAGE_DIR 0x10000
#define PG_NUM 16

#define LOW_MEM 0x100000
#define MAP_NR(addr) (((addr)-LOW_MEM) >> 12)
#define invalidate() __asm__("movl %%eax,%%cr3" ::"a"(PAGE_DIR))

#include "sched.h"

extern void mm_init();
extern unsigned long get_free_page();
extern int free_page_tables(unsigned long from, unsigned long size);
extern int copy_page_tables(unsigned long from, unsigned long to, long size);
extern unsigned long get_data_base(struct task_struct *s);
extern void free_page(unsigned long addr);
#endif