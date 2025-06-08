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
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "vm/frame.h"

unsigned SPT_hash(const struct hash_elem *e, void *aux) {
  struct page *p = hash_entry(e, struct page, SPT_elem);

  // hash_bytes conveniently returns appropriate hash with given size,
  // which is better than our previous hash function, pt_no.
  return hash_bytes(&p->page_addr, sizeof(p->page_addr));
}

bool SPT_less(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
  struct page *p_a = hash_entry(a, struct page, SPT_elem);
  struct page *p_b = hash_entry(b, struct page, SPT_elem);
  return p_a->page_addr < p_b->page_addr;
}

// Function used in SPT_destroy
void SPT_destructor(struct hash_elem *e, void *aux) {
  if (e != NULL) {
    struct page *p = hash_entry(e, struct page, SPT_elem);
    if (p->frame_addr != NULL && !p->is_swapped) {
      pagedir_clear_page(thread_current()->pagedir, p->page_addr);
      if (find_frame(p->frame_addr)) frame_free(p->frame_addr);
    }
    free(p);
  }
}

void SPT_init() { hash_init(&thread_current()->SPT, SPT_hash, SPT_less, NULL); }

struct page *SPT_search(struct thread *owner, void *page_addr) {
  struct page temp;
  temp.page_addr = page_addr;
  struct hash_elem *e = hash_find(&(owner->SPT), &temp.SPT_elem);
  if (e != NULL) {
    struct page *p = hash_entry(e, struct page, SPT_elem);
    return p;
  } else {
    return NULL;
  }
}

void SPT_insert(struct file *f, off_t ofs, void *page_addr, void *frame_addr,
                size_t read_bytes, size_t zero_bytes, bool writable,
                enum page_purpose purpose) {
  if (SPT_search(thread_current(), page_addr) != NULL) {
    printf("EXIST NO!!!!\n");
    return;
  }
  // printf("SPT insert: page %p into thread %s\n", page_addr,
  //        thread_current()->name);
  struct page *p;
  p = malloc(sizeof(struct page));
  p->page_file = f;
  p->ofs = ofs;
  p->page_addr = page_addr;
  p->frame_addr = frame_addr;
  p->read_bytes = read_bytes;
  p->zero_bytes = zero_bytes;
  p->is_writable = writable;
  p->is_swapped = false;
  p->purpose = purpose;
  p->swap_i = 0;

  struct frame *frame = find_frame(frame_addr);
  if (frame) {
    frame->page_addr = page_addr;  // I forgot this...
    frame->is_evictable = true;
  }
  hash_insert(&thread_current()->SPT, &p->SPT_elem);
}

void SPT_remove(void *page_addr) {
  struct page temp;
  temp.page_addr = page_addr;
  struct hash_elem *e = hash_delete(&thread_current()->SPT, &temp.SPT_elem);
  if (e != NULL) {
    struct page *p = hash_entry(e, struct page, SPT_elem);
    free(p);
  }
}

void SPT_destroy() { hash_destroy(&thread_current()->SPT, SPT_destructor); }

/*
// Selects victim using the page replacement algorithm
// called the second chance algorithm
struct page* get_victim() {
  // accessed bit of pte :  PTE_A
  // dirty bit of pte :     PTE_D

}
*/
