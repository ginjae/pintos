#include "vm/swap.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "lib/kernel/bitmap.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/synch.h"

#define SEC_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)
#define FREE false
#define FILLED true

// Swap disk.
static struct block *swap_disk;

// Mapping of swap disk. false: empty, true: filled.
static struct bitmap *disk_map;

void SD_init() {
  swap_disk = block_get_role(BLOCK_SWAP);
  if (!swap_disk) {
    printf("swap.c: Swap disk does not exist.\n");
    return;
  }

  size_t dsk_size = block_size(swap_disk);

  disk_map = bitmap_create(dsk_size);
  if (!disk_map) {
    printf("swap.c: bitmap init failed.\n");
    return;
  }
}

void SD_read(size_t idx, void *page) {
  bitmap_set_multiple(disk_map, idx, SEC_PER_PAGE, FREE);
  size_t i;
  for (i = idx; i < idx + SEC_PER_PAGE; i++) {
    block_read(swap_disk, i, page + BLOCK_SECTOR_SIZE * (i - idx));
  }
}

size_t SD_write(size_t idx_, void *page) {
  size_t idx = idx_;
  if (idx == 0) {
    idx = bitmap_scan_and_flip(disk_map, 0, SEC_PER_PAGE, FREE);
  } else
    bitmap_set_multiple(disk_map, idx, SEC_PER_PAGE, FILLED);

  size_t i;
  for (i = idx; i < idx + SEC_PER_PAGE; i++) {
    block_write(swap_disk, i, page + BLOCK_SECTOR_SIZE * (i - idx));
  }
  return idx;
}
