#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"

static void syscall_handler(struct intr_frame*);

static struct file* fd_table[128]; // FIXME: 128 is arbitrary.

void
syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

int open(const char *file) {
  int i;
  struct file* f = filesys_open(file);
  if (f == NULL) {
    return -1; // error
  }
  for (i = 0; i < 128; i++) {
    if (fd_table[i] == NULL) {
      fd_table[i] = f;
      // Note: table index = fd - 2
      return i + 2;
    }
  }
  return -1;
}

void close(int fd) {
  int table_index = fd - 2;
  if (table_index < 0 || table_index > 127 || fd_table[table_index] == NULL) {
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_exit();
    return;
  }
  else {
    file_close(fd_table[table_index]);
    fd_table[table_index] = NULL;
    return;
  }
}

int write(int fd, void *buffer, unsigned size) {
  if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  return -1;
}

/* A function which checks if a virtual address is valid.
   If not, the thread exits with exit code -1 */
void check_valid(void *addr) {
  uint32_t* current_pd = thread_current()->pagedir;
  if (!addr || !is_user_vaddr(addr) || !pagedir_get_page(current_pd, addr)) {
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_exit();
    return;
  }
}

static void
syscall_handler(struct intr_frame* f)
{
  // Before handling system call:
  // Check if the stack pointer is valid (sc-bad-sp)
  check_valid(f->esp);

  // Check if arguments are valid (sc-bad-arg)
  // -> in the cases of switch statement

  // if (!is_user_vaddr(f->esp + 4)) {
  //   printf("%s: exit(%d)\n", thread_name(), -1);
  //   thread_exit();
  //   return;
  // }

  // handling system call
  switch (*(uint32_t *)f->esp) {

  case SYS_HALT:
    shutdown_power_off();
    break;

  case SYS_EXIT:
    check_valid(f->esp + 4);
    printf("%s: exit(%d)\n", thread_name(), *(uint32_t *)(f->esp + 4));
    thread_exit();
    break;

  case SYS_CREATE:
    if (!pagedir_get_page((uint32_t *)thread_current()->pagedir, (const void *)*(uint32_t *)(f->esp + 4))) {
      printf("%s: exit(%d)\n", thread_name(), -1);
      f->eax = 0; // return 0 (false)
      thread_exit();
      return;
    }
    check_valid(f->esp + 4);
    check_valid(f->esp + 8);
    if (filesys_create((const char *)*(uint32_t *)(f->esp + 4), (unsigned)*((uint32_t *)(f->esp + 8)))) {
      f->eax = 1; // return 1 (true)
    }
    else {
      f->eax = 0; // return 0 (false)
    }
    break;

  case SYS_OPEN:
    if (!pagedir_get_page((uint32_t *)thread_current()->pagedir, (const void *)*(uint32_t *)(f->esp + 4))) {
      printf("%s: exit(%d)\n", thread_name(), -1);
      f->eax = -1; // return -1 (error)
      thread_exit();
      return;
    }
    check_valid(f->esp + 4);
    f->eax = open((const char *)*(uint32_t *)(f->esp + 4));
    break;

  case SYS_CLOSE:
    check_valid(f->esp + 4);
    close((int)*(uint32_t *)(f->esp + 4));
    break;

  case SYS_WRITE:
    check_valid(f->esp + 4);
    check_valid(f->esp + 8);
    check_valid(f->esp + 12);
    f->eax = write((int)*(uint32_t *)(f->esp + 4),
              (void *)*(uint32_t *)(f->esp + 8),
              (unsigned)*((uint32_t *)(f->esp + 12)));
    break;

  default:
    break;
  }
}
