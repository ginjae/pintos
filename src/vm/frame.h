#ifndef FRAME_H
#define FRAME_H

#include <palloc.h>
#include <stddef.h>

// Divide the entire memory into frames.
// Note: Considering using while loop and palloc_get_page()
void frame_table_init(size_t user_frame_limit);

// Note: our goal is to replace palloc functions in process.c
//       I think load_segment() and install_page() should be considered.
void* frame_alloc(enum palloc_flags, uint8_t* upage);

void frame_free(void*);

#endif /* FRAME_H */
