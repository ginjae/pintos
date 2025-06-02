// Includes are copied from palloc.c
// I suspect we should reference a lot of things from that file, but not sure.

#include "vm/frame.h"

#include <bitmap.h>
#include <debug.h>
#include <inttypes.h>
#include <list.h>
#include <palloc.h>
#include <round.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "threads/loader.h"
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

/* Default implementation for frame. (without swap or evict, etc.)*/
struct frame {
  void* frame_addr;             // allocated frame's address.
  void* page_addr;              // if it's allocated, to which page?
  struct thread* owner_thread;  // what process owns this frame?
};

/* Frame table that keeps track of all available frames. */
static struct list frame_table;

// 2. Define frame table allocator. (say falloc_get_frame()...?)
//    to replace palloc_get_page() in process.c

void frame_table_init(size_t user_frame_limit) {
  // initialize list frame_table.
}

void* frame_alloc(enum palloc_flags, uint8_t* upage,
                  struct thread* owner_thread) {
  // Pseudocode (not sure!)

  /*
  i) create new struct frame
  ii) assign palloc's result to member void* frame_addr
  iii) assign address upage to member void* page_addr
  iv) assign argument owner_thread to member owner_thread
  v) push struct into static struct list frame_table.
  */

  // note: iv) might be unnecessary since we can use thread_current()
  //       but just in case!
}
