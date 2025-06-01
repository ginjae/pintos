// Includes are copied from palloc.c
// I suspect we should reference a lot of things from that file, but not sure.

#include "vm/frame.h"

#include <bitmap.h>
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "threads/loader.h"
#include "threads/synch.h"
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

// 2. Define frame table allocator. (say falloc_get_frame()...?)
//    to replace palloc_get_page() in process.c
