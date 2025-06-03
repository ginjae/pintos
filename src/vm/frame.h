#ifndef FRAME_H
#define FRAME_H

#include <stddef.h>

#include "threads/palloc.h"

// Initialize list object named frame_table
void frame_table_init(size_t user_frame_limit UNUSED);

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
