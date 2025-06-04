#ifndef FRAME_H
#define FRAME_H

#include <list.h>
#include <stddef.h>
#include <stdint.h>

#include "threads/palloc.h"

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

// Initialize list object named frame_table
void frame_table_init(size_t user_frame_limit);

// Find frame with physical address. (call this with frame_lock!)
struct frame* find_frame(void* kpage);

// Update frame's last accessed time. (Probably needed later.)
void frame_time_update(void* kpage);

// Allocate frame & update frame table.
void* frame_alloc(enum palloc_flags);

// Set upage of corresponding struct frame.
void frame_update_upage(void* upage, void* kpage);

// Free frame with corresponding physical address.
void frame_free(void* kpage);

#endif /* vm/frame.h */
