#ifndef SWAP_H
#define SWAP_H
#include <stddef.h>

#include "devices/block.h"

void SD_init();

// Read PGSIZE bytes of data from the swap_disk to page
void SD_read(size_t idx, void* page);

// Write PGSIZE bytes of data from the page to the swap_disk,
// and return its index.
size_t SD_write(void* page);

#endif /* vm/swap.h */
