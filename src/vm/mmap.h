#ifndef MMAP_H
#define MMAP_H

#include <stdio.h>
#include <list.h>
#include <stdint.h>

struct mapping {
    int id;                 /* Mapping id */
    void *addr;             /* Starting virtual address to store */
    int32_t size;           /* File length */
    struct file *file;      /* Target file */
    struct list_elem elem;  /* List element for mmap_table in process */
    struct list pages;      /* List of pages that the mapping allocated */
};

struct mapping *find_mapping_addr(struct list* mmap_table, void* addr);
struct mapping *find_mapping_id(struct list* mmap_table, int id);

#endif  /* vm/mmap.h */