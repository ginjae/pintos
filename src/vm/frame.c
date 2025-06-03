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
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

// TODO

// 1. Define frame table & frame.

//    OS must maintain the following informations:
//      a) Which frames are allocated
//      b) Which frames are available
//      c) How many total frames are there

//    Frame table has one entry for each physical page frame, including:
//      a) Whether each frame is free or allocated
//      b) If it is allocated, to which page of which process(es)

/* Default implementation for frame. (without swap or evict, etc.) */
struct frame {
  void* frame_addr;              // allocated frame's address. (=kpage)
  void* page_addr;               // virtual address pointing to frame. (=upage)
  struct thread* owner_thread;   // Process(thread) who owns this frame
  struct list_elem ftable_elem;  // list element for frame table list
  int64_t access_time;           // last accessed time (probably needed later)
};

/* Frame table that keeps track of all available frames. */
static struct list frame_table;

/* Lock for frame_alloc, which is critical section. */
static struct lock frame_lock;

// 2. Define frame table allocator.
//    to replace palloc_get_page(PAL_USER) in process.c

void frame_table_init(size_t user_frame_limit) {
  list_init(&frame_table);  // initialize list frame_table.
  lock_init(&frame_lock);   // initialize frame lock.
}

struct frame* find_frame(void* kpage) {
  // frame table is empty: return NULL
  if (list_empty(&frame_table)) return NULL;

  struct list_elem* e;
  struct frame* f;

  for (e = list_begin(&frame_table); e != list_end(&frame_table);
       e = list_next(e)) {
    f = list_entry(e, struct frame, ftable_elem);
    if (f->frame_addr == kpage) return f;
  }

  // No such frame is found: return NULL.
  return NULL;
}

void frame_time_update(void* kpage) {
  lock_acquire(&frame_lock);
  struct frame* f = find_frame(kpage);
  f->access_time = timer_ticks();
  lock_release(&frame_lock);
}

void* frame_alloc(enum palloc_flags flags) {
  // i) create new struct frame
  struct frame* new_frame = malloc(sizeof(struct frame));

  // ii) assign palloc's result to member void* frame_addr
  uint8_t* kpage = palloc_get_page(flags);
  if (!kpage) {
    free(new_frame);
    return NULL;
  }
  new_frame->frame_addr = kpage;

  // iii) assign current thread to member owner_thread
  new_frame->owner_thread = thread_current();

  // iv) record access time
  new_frame->access_time = timer_ticks();

  // v) push struct into static struct list frame_table.
  //    since this is the critical section, use lock!
  lock_acquire(&frame_lock);
  // FIXME: We probably should use some kind of sorting...? Not sure.
  list_push_back(&frame_table, &new_frame->ftable_elem);
  lock_release(&frame_lock);

  // vi) return kernel virtual address (physical address)
  return kpage;
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
  struct list_elem* e;
  struct frame* f;

  lock_acquire(&frame_lock);

  // Note: list_empty is critical section, so it requires lock.
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
  lock_release(&frame_lock);
}
