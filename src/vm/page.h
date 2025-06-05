#ifndef PAGE_H
#define PAGE_H

#include <stddef.h>
#include <hash.h>
#include <list.h>

#include "threads/palloc.h"

/*

SPT acts like a "to-do list": it stores info that page fault handler needs to
know so that it can manage memory request that caused page fault.

This "metadata" includes:

- User virtual address (=upage) (used as the key in the SPT)
- Physical address (=kpage)
- How much data should be loaded (remaining part should filled with zero)
- Writiable or not (see install_page() in process.c)
- Index for backing store in case of it is swapped
- Purpose for this page (file / stack / swapped)
  -> this is required for handling eviction (ex. write back)

*/

struct page {
  void *page_addr;   // upage
  void *frame_addr;  // kpage

  bool is_writable;  // is writing on this page allowed?
//   size_t swap_i;     // index for backing store (swapped page end up there)
//   enum page_purpose purpose;  // Purpose for this page

//   struct file *page_file;  // file for read (if purpose == FOR_FILE)
//   size_t load_bytes;       // size of read
//   size_t zero_bytes;       // size of remaining page (should be zeroed)

  struct hash_elem SPT_elem;  // hash elem for hash SPT
};

struct hash SPT;

// enums for specifying page's purpose
enum page_purpose { FOR_FILE = 0, FOR_STACK = 1, SWAPPED = 2 };

// Initialize list object named frame_table. Call this in load()!
void SPT_init(size_t user_frame_limit);

void SPT_insert(void *page_addr, void *frame_addr);
void SPT_remove(void *page_addr);

// struct page* get_victim();

#endif /* vm/page.h */
