#ifndef PAGE_H
#define PAGE_H

#include <stddef.h>

#include "threads/palloc.h"

// enums for specifying page's purpose
enum page_purpose { FOR_FILE = 0, FOR_STACK = 1, SWAPPED = 2 };

// Initialize list object named frame_table. Call this in load()!
void SPT_init(size_t user_frame_limit);

#endif /* vm/page.h */
