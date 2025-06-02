#ifndef FRAME_H
#define FRAME_H

#include <stddef.h>

#include "threads/palloc.h"
#include "threads/thread.h"

// Define frame table & frame.

// OS must maintain the following informations:
//  a) Which frames are allocated
//  b) Which frames are available
//  c) How many total frames are there

// Frame table has one entry for each physical page frame, including:
//  a) Whether each frame is free or allocated
//  b) If it is allocated, to which page of which process(es)

/* Default implementation for frame. (without swap or evict, etc.)*/
struct frame {
    struct list_elem elem;        // List element for frame_table
    void* frame_addr;             // allocated frame's physical address.
    void* page_addr;              // virtual address
    struct thread* owner_thread;  // what process owns this frame?
};

/* Frame table that keeps track of all available frames. */
static struct list frame_table;

// Divide the entire memory into frames.
// Note: Considering using while loop and palloc_get_page()
void frame_table_init(size_t user_frame_limit);

void frame_add(void* frame_addr, void* page_addr, struct thread* owner_thread);

void frame_free(void* page_addr);

void* get_free_frame();

#endif /* vm/frame.h */
