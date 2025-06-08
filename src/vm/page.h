#ifndef PAGE_H
#define PAGE_H

#include <hash.h>
#include <list.h>
#include <stddef.h>

#include "filesys/off_t.h"
#include "threads/palloc.h"
#include "threads/thread.h"

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

// enums for specifying page's purpose
enum page_purpose { FOR_FILE = 0, FOR_STACK = 1, FOR_MMAP = 2 };

struct page {
  void *page_addr;   // upage
  void *frame_addr;  // kpage

  bool is_writable;  // is writing on this page allowed?
  size_t swap_i;     // index for swap disk (swapped page end up there)
  bool is_swapped;   // true if this page is in swap_disk, false otherwise.
  enum page_purpose purpose;  // Purpose for this page

  /* File-related members */
  struct file *page_file;  // file for read (if purpose == FOR_FILE)
  off_t ofs;               // file offset.
  size_t read_bytes;       // size of read bytes.
  size_t zero_bytes;       // size of remaining page (should be zeroed)

  struct hash_elem SPT_elem;  // hash elem for hash SPT
};

// Initialize list object named frame_table. Call this in load()!
void SPT_init();

// Find page using page address as key.
struct page *SPT_search(struct thread *owner, void *page_addr);

// Insert new page "to-do list" into SPT.
// Arguments are copied from load_segment() in process.c
void SPT_insert(struct file *f, off_t ofs, void *page_addr, void *frame_addr,
                size_t read_bytes, size_t zero_bytes, bool writable,
                enum page_purpose purpose);

void SPT_remove(void *page_addr);

void SPT_destroy();

#endif /* vm/page.h */
