#ifndef MMAP_H
#define MMAP_H

#include <stdio.h>
#include <list.h>
#include <stdint.h>

struct mapping {
    int id;
    void *addr;
    int32_t size;
    struct file *file;
    int fd;
    struct list_elem elem;
};

struct mapping *find_mapping_addr(struct list* mmap_table, void* addr);
struct mapping *find_mapping_id(struct list* mmap_table, int id);

#endif  /* vm/mmap.h */