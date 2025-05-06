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

// static struct file* fd_table[128]; // FIXME: 128 is arbitrary.

void
syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

int open(const char* file) {
  struct file** fd_table = thread_current()->fd_table;
  int i;
  struct file* f = filesys_open(file);
  if (f == NULL) {
    return -1; // error
  }
  for (i = 2; i < 130; i++) {
    if (fd_table[i] == NULL) {
      fd_table[i] = f;
      // Note: table index = fd - 2
      return i;
    }
  }
  return -1;
}

void close(int fd) {
  struct file** fd_table = thread_current()->fd_table;
  if (fd < 2 || fd > 129 || fd_table[fd] == NULL) {
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_exit();
    return;
  }
  else {
    file_close(fd_table[fd]);
    fd_table[fd] = NULL;
    return;
  }
}

int write(int fd, void* buffer, unsigned size) {
  if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  return -1;
}

/* A function which checks if a virtual address is valid.
   If not, the thread exits with exit code -1 */
void check_valid(void* addr) {
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

  // handling system call
  switch (*(uint32_t*)f->esp) {

  case SYS_HALT:
    shutdown_power_off();
    break;

  case SYS_EXIT:
    check_valid(f->esp + 4);
    printf("%s: exit(%d)\n", thread_name(), *(uint32_t*)(f->esp + 4));
    thread_exit();
    break;

  case SYS_EXEC:
    // pid_t exec (const char *cmd_line)

    // Runs the executable whose name is given in cmd line, passing any given arguments,
    // and returns the new process’s program id(pid). Must return pid - 1,

    break;

  case SYS_WAIT:
    // int wait (pid t pid)

    // Waits for a child process pid and retrieves the child’s exit status.
    // Implementing this system call requires considerably more work than any of the rest.

    break;

  case SYS_CREATE:
    if (!pagedir_get_page((uint32_t*)thread_current()->pagedir, (const void*)*(uint32_t*)(f->esp + 4))) {
      printf("%s: exit(%d)\n", thread_name(), -1);
      f->eax = 0; // return 0 (false)
      thread_exit();
      return;
    }
    check_valid(f->esp + 4);
    check_valid(f->esp + 8);
    if (filesys_create((const char*)*(uint32_t*)(f->esp + 4), (unsigned)*((uint32_t*)(f->esp + 8)))) {
      f->eax = 1; // return 1 (true)
    }
    else {
      f->eax = 0; // return 0 (false)
    }
    break;

  case SYS_REMOVE:
    // bool remove (const char *file)

    // Deletes the file called file. Returns true if successful, false otherwise. A file may be
    // removed regardless of whether it is open or closed, and removing an open file does
    // not close it.

    break;

  case SYS_OPEN:
    if (!pagedir_get_page((uint32_t*)thread_current()->pagedir, (const void*)*(uint32_t*)(f->esp + 4))) {
      printf("%s: exit(%d)\n", thread_name(), -1);
      f->eax = -1; // return -1 (error)
      thread_exit();
      return;
    }
    check_valid(f->esp + 4);
    f->eax = open((const char*)*(uint32_t*)(f->esp + 4));
    break;

  case SYS_FILESIZE:
    // int filesize (int fd)

    // Returns the size, in bytes, of the file open as fd.

    break;

  case SYS_READ:
    // int read(int fd, void* buffer, unsigned size)

    // Reads size bytes from the file open as fd into buffer. Returns the number of bytes
    // actually read(0 at end of file), or -1 if the file could not be read(due to a condition
    // other than end of file).Fd 0 reads from the keyboard using input_getc().

    break;

  case SYS_WRITE:
    // int write (int fd, const void *buffer, unsigned size)

    // Writes size bytes from buffer to the open file fd. Returns the number of bytes actually
    // written, which may be less than size if some bytes could not be written.

    check_valid(f->esp + 4);
    check_valid(f->esp + 8);
    check_valid(f->esp + 12);
    f->eax = write((int)*(uint32_t*)(f->esp + 4),
      (void*)*(uint32_t*)(f->esp + 8),
      (unsigned)*((uint32_t*)(f->esp + 12)));

    break;

  case SYS_SEEK:
    // void seek (int fd, unsigned position)

    // Changes the next byte to be read or written in open file fd to position, expressed in
    // bytes from the beginning of the file.

    break;

  case SYS_TELL:
    // unsigned tell (int fd)

    // Returns the position of the next byte to be read or written in open file fd, expressed
    // in bytes from the beginning of the file.

    break;

  case SYS_CLOSE:
    check_valid(f->esp + 4);
    close((int)*(uint32_t*)(f->esp + 4));
    break;

  default:
    break;
  }
}
