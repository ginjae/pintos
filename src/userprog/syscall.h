#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
int open(const char *file);
void close(int fd);
int write(int fd, void *buffer, unsigned size);
void check_valid(void *addr);

#endif /* userprog/syscall.h */
