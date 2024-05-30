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

// implement
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  // thread_exit ();
  // check validation of the pointers in parameter list
  if (f->)

  int syscall_number = 0;
  switch (syscall_number)
  {
  case SYS_HALT:
  halt();
  case SYS_EXIT:
  exit();
  case SYS_EXEC:
  exec();
  case SYS_WAIT:
  wait();
  case SYS_CREATE:
  create();
  case SYS_REMOVE:
  remove(f);
  case SYS_OPEN:
  open();
  case SYS_FILESIZE:
  filesize();
  case SYS_READ:
  read();
  case SYS_WRITE:
  write();
  case SYS_SEEK:
  seek();
  case SYS_TELL:
  tell();
  case SYS_CLOSE:
  close();
  }
}
