#include "vm/swap.h"

#include <debug.h>
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

// Lock.
static struct lock swap_lock;

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
  lock_init(&swap_lock);
}

void SD_read(size_t idx, void *page) {
  lock_acquire(&swap_lock);
  if (idx == BITMAP_ERROR)
    PANIC("BUG: SD_read called with BITMAP_ERROR. frame addr: %p\n", page);
  ASSERT(!bitmap_contains(disk_map, idx, SEC_PER_PAGE, FREE));

  // printf("SD_read is reading idx: %zu\n", idx);
  size_t i;
  for (i = idx; i < idx + SEC_PER_PAGE; i++) {
    block_read(swap_disk, i, page + BLOCK_SECTOR_SIZE * (i - idx));
  }
  if (idx != 0) bitmap_set_multiple(disk_map, idx, SEC_PER_PAGE, FREE);
  lock_release(&swap_lock);
}

size_t SD_write(void *page) {
  size_t idx;
  lock_acquire(&swap_lock);
  idx = bitmap_scan_and_flip(disk_map, 0, SEC_PER_PAGE, FREE);
  // printf("bitmap_scan_and_flip returned: %zu\n", idx);
  if (idx == BITMAP_ERROR) {
    // printf("Somehow, you fucked up. idx=%zu\n", idx);
    return idx;
  }

  size_t i;
  for (i = idx; i < idx + SEC_PER_PAGE; i++) {
    block_write(swap_disk, i, page + BLOCK_SECTOR_SIZE * (i - idx));
  }
  lock_release(&swap_lock);

  return idx;
}
