// Includes are copied from palloc.c
// I suspect we should reference a lot of things from that file, but not sure.

#include "vm/frame.h"

#include <bitmap.h>
#include <debug.h>
#include <inttypes.h>
#include <list.h>
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
#include "vm/page.h"
#include "vm/swap.h"

void frame_table_init(size_t user_frame_limit) {
  list_init(&frame_table);  // initialize list frame_table.
  lock_init(&frame_lock);   // initialize frame lock.
}

struct frame* find_frame(void* kpage) {
  // frame table is empty: return NULL
  if (list_empty(&frame_table)) return NULL;

  struct list_elem* e;
  struct frame* f;
  // lock_acquire(&frame_lock);

  for (e = list_begin(&frame_table); e != list_end(&frame_table);
       e = list_next(e)) {
    f = list_entry(e, struct frame, ftable_elem);
    if (f->frame_addr == kpage) return f;
  }

  // lock_release(&frame_lock);

  // No such frame is found: return NULL.
  return NULL;
}

struct frame* find_victim() {
  // frame table is empty: return NULL
  if (list_empty(&frame_table)) return NULL;
  if (!ft_cursor) ft_cursor = list_begin(&frame_table);

  struct frame* f;
  uint32_t* pagedir;
  size_t loop_lim = 10000;
  size_t counter = 0;

  struct list_elem* victim_cursor = NULL;

  lock_acquire(&frame_lock);

  while (counter < loop_lim) {
    f = list_entry(ft_cursor, struct frame, ftable_elem);
    pagedir = f->owner_thread->pagedir;

    if (!f->is_evictable || pagedir_is_accessed(pagedir, f->page_addr)) {
      ft_cursor = list_next(ft_cursor);
      if (ft_cursor == list_end(&frame_table))
        ft_cursor = list_begin(&frame_table);
      if (f->is_evictable) pagedir_set_accessed(pagedir, f->page_addr, false);
      counter++;
    } else {
      void* page_addr = f->page_addr;
      victim_cursor = ft_cursor;
      ft_cursor = list_next(ft_cursor);
      if (ft_cursor == list_end(&frame_table))
        ft_cursor = list_begin(&frame_table);
      f = list_entry(victim_cursor, struct frame, ftable_elem);
      break;
    }
  }

  lock_release(&frame_lock);

  return f;
}

void swap_frame(struct frame* victim) {
  // Assume that the victim is removed from the frame table.
  void* page_addr = victim->page_addr;
  void* frame_addr = victim->frame_addr;
  struct page* page = SPT_search(page_addr);
  if (!page) {
    palloc_free_page(frame_addr);
    victim->frame_addr = NULL;
    return;
  }
  victim->is_evictable = false;

  enum page_purpose pur = page->purpose;
  size_t idx = page->swap_i;
  struct thread* owner = victim->owner_thread;

  switch (pur) {
    case FOR_FILE:
      if (pagedir_is_dirty(owner->pagedir, page_addr)) {
        page->swap_i = SD_write(idx, frame_addr);
        pagedir_set_dirty(owner->pagedir, page_addr, false);
      }
      page->is_swapped = true;
      break;

    case FOR_STACK:
      page->swap_i = SD_write(idx, frame_addr);
      page->is_swapped = true;
      break;

    case FOR_MMAP:
      if (pagedir_is_dirty(owner->pagedir, page_addr)) {
        file_write_at(page->page_file, frame_addr, PGSIZE, page->ofs);
        pagedir_set_dirty(owner->pagedir, page_addr, false);
      }
      page->is_swapped = false;
      break;
  }

  page->frame_addr = NULL;

  pagedir_clear_page(owner->pagedir, page_addr);
  palloc_free_page(frame_addr);
  free(victim);
}

void* frame_alloc(enum palloc_flags flags) {
  // i) create new struct frame
  struct frame* new_frame = malloc(sizeof(struct frame));
  struct frame* victim = NULL;

  new_frame->is_evictable = false;

  // ii) assign palloc's result to member void* frame_addr
  uint8_t* kpage = palloc_get_page(flags);
  while (!kpage) {
    // have to use page replacement algorithm
    victim = find_victim();
    lock_acquire(&frame_lock);
    list_remove(&(victim->ftable_elem));
    lock_release(&frame_lock);
    swap_frame(victim);
    kpage = palloc_get_page(flags);
    new_frame->is_evictable = true;
  }

  new_frame->frame_addr = kpage;

  // iii) assign current thread to member owner_thread
  new_frame->owner_thread = thread_current();

  // iv) record access time (redundant for second chance)
  // new_frame->access_time = timer_ticks();

  // v) push struct into static struct list frame_table.
  //    since this is the critical section, use lock!
  // lock_acquire(&frame_lock);
  list_push_back(&frame_table, &new_frame->ftable_elem);
  if (!ft_cursor) ft_cursor = list_begin(&frame_table);
  // lock_release(&frame_lock);

  // vi) return kernel virtual address (physical address)
  return kpage;
}

void frame_add(void* page_addr, void* frame_addr, struct thread* t) {
  struct list_elem* e;
  struct frame* f;
  lock_acquire(&frame_lock);

  for (e = list_begin(&frame_table); e != list_end(&frame_table);
       e = list_next(e)) {
    f = list_entry(e, struct frame, ftable_elem);
    if (f->frame_addr == frame_addr) {
      break;
    }
    f = NULL;
  }
  if (f != NULL) {
    f->page_addr = page_addr;
    f->owner_thread = t;
  } else {
    struct frame* new_frame = malloc(sizeof(struct frame));
    new_frame->page_addr = page_addr;
    new_frame->frame_addr = frame_addr;
    new_frame->owner_thread = t;
    list_push_back(&frame_table, &new_frame->ftable_elem);
  }

  lock_release(&frame_lock);
}

void frame_update_upage(void* upage, void* kpage) {
  // Reading frame table has potential race condition,
  // so we use lock here again!
  lock_acquire(&frame_lock);
  struct frame* f = find_frame(kpage);
  if (f) f->page_addr = upage;
  lock_release(&frame_lock);
}

void frame_free(void* kpage) {
  if (!kpage) return;
  struct list_elem* e;
  struct frame* f;

  // lock_acquire(&frame_lock);

  if (list_empty(&frame_table)) {
    lock_release(&frame_lock);
    return;
  }

  for (e = list_begin(&frame_table); e != list_end(&frame_table);
       e = list_next(e)) {
    f = list_entry(e, struct frame, ftable_elem);
    if (f->frame_addr == kpage) {
      // update frame table
      list_remove(e);

      // allocated by palloc_get_page(PAL_USER)
      palloc_free_page(f->frame_addr);

      // allocated by malloc(sizeof(struct frame))
      free(f);

      break;
    }
  }
  // lock_release(&frame_lock);
}
