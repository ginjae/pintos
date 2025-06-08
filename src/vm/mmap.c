#include "vm/mmap.h"

#include <list.h>

struct mapping *find_mapping_addr(struct list* mmap_table, void* addr) {
	struct list_elem *e;
	struct mapping *m;
	
	for (e = list_begin(mmap_table); e != list_end(mmap_table); e = list_next(e)) {
		m = list_entry(e, struct mapping, elem);
		if (m->addr <= addr && addr < m->addr + m->size)
			return m;
	}
	return NULL;
}

struct mapping *find_mapping_id(struct list* mmap_table, int id) {
	struct list_elem *e;
	struct mapping *m;
	
	for (e = list_begin(mmap_table); e != list_end(mmap_table); e = list_next(e)) {
		m = list_entry(e, struct mapping, elem);
		if (m->id == id)
			return m;
	}
	return NULL;
}
