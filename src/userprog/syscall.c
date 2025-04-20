#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  // printf("\n\n");
  // hex_dump(f->esp, f->esp, 100, 1);
  switch (*(uint32_t *)f->esp) {
    case SYS_HALT:
      shutdown_power_off();
      break;
    case SYS_EXIT:
      printf ("%s: exit(%d)\n", thread_name(), *(uint32_t *)(f->esp + 4));
      thread_exit();
      break;
    case SYS_CREATE:
      break;
    case SYS_OPEN:
      break;
    case SYS_CLOSE:
      break;
    case SYS_WRITE:
      write((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8),
        (unsigned)*((uint32_t *)(f->esp + 12)));
      break;
    default:
      break;
  }
}

int write(int fd, void *buffer, unsigned size) {
  if (fd == 1) {
    putbuf(buffer, size);
    return size;  // FIXME
  }
  return -1;
}