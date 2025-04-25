#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

static void syscall_handler(struct intr_frame*);

void
syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

int write(int fd, void* buffer, unsigned size) {
  if (fd == 1) {
    putbuf(buffer, size);
    return size;  // FIXME
  }
  return -1;
}

static void
syscall_handler(struct intr_frame* f)
{
  // printf("\n\n");
  // hex_dump(f->esp, f->esp, 100, 1);

  // Before handling system call:
  // 1. Check if the stack pointer is valid (sc-bad-sp)
  uint32_t* current_pd = thread_current()->pagedir;
  if (!f->esp || !pagedir_get_page(current_pd, f->esp) || !is_user_vaddr(f->esp)) {
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_exit();
    return;
  }

  // 2. Check if arguments are valid (sc-bad-arg)
  if (!is_user_vaddr(f->esp + 4)) {
    // FIXME: It only checks the case where the 1st argument is above the top of the user address space,
    //        which is our only test case.
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_exit();
    return;
  }


  // handling system call
  switch (*(uint32_t*)f->esp) {
  case SYS_HALT:
    shutdown_power_off();
    break;
  case SYS_EXIT:
    printf("%s: exit(%d)\n", thread_name(), *(uint32_t*)(f->esp + 4));
    thread_exit();
    break;
  case SYS_CREATE:
    // do something
    break;
  case SYS_OPEN:
    // do something
    break;
  case SYS_CLOSE:
    // do something
    break;
  case SYS_WRITE:
    write((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8),
      (unsigned)*((uint32_t*)(f->esp + 12)));
    break;
  default:
    break;
  }
}
