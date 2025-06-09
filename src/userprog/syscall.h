#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"
#include "threads/thread.h"

void syscall_init(void);
int open(const char* file);
int filesize(int fd);
int read(int fd, void* buffer, unsigned size);
int write(int fd, void* buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);

void check_valid(void* addr);
int mmap(int fd, void* addr);
void munmap_write(struct thread* t, int mapping, bool unmap);
void munmap_free(struct thread* t, int mapping);
void munmap(int mapping);

struct lock filesys_lock;

#endif /* userprog/syscall.h */
