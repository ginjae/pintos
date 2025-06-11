#ifndef FRAME_H
#define FRAME_H

#include <list.h>
#include <stddef.h>
#include <stdint.h>

#include "filesys/off_t.h"
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
  void* page_addr;               // virtual address pointing to page. (=upage)
  struct thread* owner_thread;   // Process(thread) who owns this frame
  struct list_elem ftable_elem;  // list element for frame table list
  bool is_evictable;             // true iff the corresponding SPT exists.
};

/* Frame table that keeps track of all available frames. */
static struct list frame_table;

/* Lock for frame_alloc, which is critical section. */
static struct lock frame_lock;

/* Cursor to imitate circular list behavior. */
static struct list_elem* ft_cursor = NULL;

// Initialize list object named frame_table
void frame_table_init(size_t user_frame_limit);

// Find frame with physical address. (call this with frame_lock!)
struct frame* find_frame(void* kpage);

// Returns victim frame via second chance algorithm.
struct frame* find_victim();

// Swap the frame's content with the swap disk
// & update corresponding SPT's swap_i value.
void swap_frame(struct frame* victim);

// Allocate frame & update frame table.
void* frame_alloc(enum palloc_flags flags, bool is_evictable);

// Set upage of corresponding struct frame.
void frame_update_upage(void* upage, void* kpage);

// Free frame with corresponding physical address.
void frame_free(void* kpage);

#endif /* vm/frame.h */
