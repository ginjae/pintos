// Includes are copied from palloc.c
// I suspect we should reference a lot of things from that file, but not sure.

#include "vm/frame.h"

#include <bitmap.h>
#include <debug.h>
#include <inttypes.h>
#include <list.h>
#include <round.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "threads/loader.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"


// Define frame table allocator. (say falloc_get_frame()...?)
// to replace palloc_get_page() in process.c

void frame_table_init(size_t user_frame_limit) {
    // initialize list frame_table.
    list_init(&list_init);
}

void frame_add(void* frame_addr, void* page_addr, struct thread* owner_thread) {
    // check addresses

    // create new frame

    // push the frame into frame table
    
}

void frame_free(void* page_addr) {

}

void* get_free_frame() {

}
