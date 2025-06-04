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
  list_init(&eviction_queue);
}

void SPT_insert(void *page_addr, void *frame_addr) {
  struct page *p;
  p = malloc(sizeof (struct page));
  p->page_addr = page_addr;
  p->frame_addr = frame_addr;
  p->is_writable = true;
  hash_insert(&SPT, &p->SPT_elem);
}

void SPT_remove(void *page_addr) {
  struct page temp;
  temp.page_addr = page_addr;
  struct hash_elem *e = hash_delete(&SPT, &temp.SPT_elem);
  if (e != NULL) {
    struct page *p = hash_entry(e, struct page, SPT_elem);
    free(p);
  }
}

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
  list_init(&eviction_queue);
}

void SPT_insert(void *page_addr, void *frame_addr) {
  struct page *p;
  p = malloc(sizeof (struct page));
  p->page_addr = page_addr;
  p->frame_addr = frame_addr;
  p->is_writable = true;
  hash_insert(&SPT, &p->SPT_elem);
}

void SPT_remove(void *page_addr) {
  struct page temp;
  temp.page_addr = page_addr;
  struct hash_elem *e = hash_delete(&SPT, &temp.SPT_elem);
  if (e != NULL) {
    struct page *p = hash_entry(e, struct page, SPT_elem);
    free(p);
  }
}

// Selects victim using the page replacement algorithm
// called the second chance algorithm
struct page* get_victim() {
  // accessed bit of pte :  PTE_A
  // dirty bit of pte :     PTE_D

}