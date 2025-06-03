#include "vm/page.h"

#include <bitmap.h>
#include <debug.h>
#include <hash.h>
#include <inttypes.h>
#include <round.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "devices/timer.h"
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

/*

SPT acts like a "to-do list": it stores info that page fault handler needs to
know so that it can manage memory request that caused page fault.

This "metadata" includes:

- User virtual address (=upage) (used as the key in the SPT)
- Physical address (=kpage)
- How much data should be loaded (remaining part should filled with zero)
- Writiable or not (see install_page() in process.c)
- Index for backing store in case of it is swapped
- Purpose for this page (file / stack / swapped)
  -> this is required for handling eviction (ex. write back)

*/

struct page {};
