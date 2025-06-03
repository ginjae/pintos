#ifndef FRAME_H
#define FRAME_H

#include <palloc.h>
#include <stddef.h>

// Initialize list object named frame_table
void frame_table_init(size_t user_frame_limit);

// Note: our goal is to replace palloc functions in process.c
//       I think load_segment() and install_page() should be considered.
void* frame_alloc(enum palloc_flags, uint8_t* upage,
                  struct thread* owner_thread);

void frame_free(void*);

#endif /* FRAME_H */
