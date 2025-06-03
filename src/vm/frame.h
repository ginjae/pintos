#ifndef FRAME_H
#define FRAME_H

#include <stddef.h>

#include "threads/palloc.h"

// Initialize list object named frame_table
void frame_table_init(size_t user_frame_limit);

// Pick frame with just kpage address.
// Note: call this with frame_lock!
struct frame* find_frame(void* kpage);

// Note: our goal is to replace palloc functions in process.c
//       I think load_segment() and install_page() should be considered.
void* frame_alloc(enum palloc_flags);

// Set upage of corresponding struct frame.
void frame_update_upage(void* upage, void* kpage);

void frame_free(void*);

#endif /* vm/frame.h */
