#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
static void check_verify (void *address);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

void
check_verify (void * address)
{
  if (!is_user_vaddr(address) || address == NULL)
    exit (-1);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  switch (f->eax)
  {
    case SYS_CREATE:
      f->eax = create (f->edi, f->esi);
      break;
    case SYS_REMOVE:
      f->eax = remove (f->edi);
      break;
    case SYS_OPEN:
      f->eax = open (f->edi);
      break;
    case SYS_FILESIZE:
      f->eax = filesize (f->edi);
      break;
    case SYS_READ:
      f->eax = read (f->edi, f->esi, f->edx);
      break;
    case SYS_WRITE:
      f->eax = write (f->edi, f->esi, f->edx);
      break;
    case SYS_SEEK:
      seek (f->edi, f->esi);
      break;
    case SYS_TELL:
      f->eax = tell (f->edi);
      break;
    case SYS_CLOSE:
      close (f->edi);
      break;
  }
  printf ("system call!\n");
  thread_exit ();
}

bool 
create (const char *file, unsigned initial_size)
{
  check_verify (file);
  return filesys_create (file);
}

bool 
remove (const char *file)
{
  check_verify (file);
  return filesys_remove (file);
}

