#ifndef SWAP_H
#define SWAP_H
#include <stddef.h>

#include "devices/block.h"

void SD_init();

// Read PGSIZE bytes of data from the swap_disk to page
void SD_read(size_t idx, void* page);

// Write PGSIZE bytes of data from the page to the swap_disk,
// with corresponding index.
size_t SD_write(size_t idx_, void* page);

#endif /* vm/swap.h */
