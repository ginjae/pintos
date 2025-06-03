#include "vm/page.h"

#include <bitmap.h>
#include <debug.h>
#include <hash.h>
#include <inttypes.h>
#include <round.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "devices/timer.h"
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

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
  int load_bytes;    // PGSIZE - (load_size % PGSIZE) = zero region
  bool is_writable;  // is writing on this page allowed?
  size_t swap_i;     // index for backing store (swapped page end up there)
  enum page_purpose purpose;  // Purpose for this page

  struct hash_elem SPT_elem;  // hash elem for hash SPT
};

struct hash SPT;

unsigned SPT_hash(const struct hash_elem *e, void *aux) {
  struct page *p = hash_entry(e, struct page, SPT_elem);
  return pt_no(p->page_addr);
}

bool SPT_less(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
  struct page *p_a = hash_entry(a, struct page, SPT_elem);
  struct page *p_b = hash_entry(b, struct page, SPT_elem);
  return p_a->page_addr < p_b->page_addr;
}

void SPT_init(size_t user_frame_limit) {
  hash_init(&SPT, SPT_hash, SPT_less, NULL);
}
