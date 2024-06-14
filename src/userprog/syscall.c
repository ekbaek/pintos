#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);

void valid_address(const uint64_t *cur_addr)
{
	if (!(is_user_vaddr(cur_addr)) || cur_addr == NULL)
	{
		exit(-1);
	}
}

void
syscall_init (void) 
{
  lock_init (&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  switch(f->eax) {
		case SYS_HALT:
			halt();
      break;
		case SYS_EXIT:
			exit(f->edi);
      break;
		case SYS_EXEC:
			f->eax = exec(f->edi);
      break;
		case SYS_WAIT:
			f->eax = wait(f->edi);
      break;
		case SYS_CREATE:
			f->eax = create(f->edi, f->esi);
      break;		
		case SYS_REMOVE:
			f->eax = remove(f->edi);
      break;		
		case SYS_OPEN:
			f->eax = open(f->edi);
      break;		
		case SYS_FILESIZE:
			f->eax = filesize(f->edi);
      break;
		case SYS_READ:
			f->eax = read(f->edi, f->esi, f->edx);
      break;
		case SYS_WRITE:
			f->eax = write(f->edi, f->esi, f->edx);
      break;		
		case SYS_SEEK:
			seek(f->edi, f->esi);
      break;		
		case SYS_TELL:
			f->eax = tell(f->edi);
      break;		
		case SYS_CLOSE:
			close(f->edi);
      break;	
    default:
		  exit(-1);
		  break;
	}
}

bool 
create (const char *file, unsigned initial_size)
{
  valid_address (file);
  bool success = filesys_create (file, initial_size);
  if(success)
    return true;
  else
    return false;
}

bool
remove (const char *file)
{
  valid_address (file);
  bool success = filesys_remove (file);
  if(success)
    return true;
  else
    return false;
}

int
open (const char *file)
{
  valid_address (file);
  struct file *f = filesys_open (file);

  if (f == NULL)
    return -1;
  
  struct thread *cur = thread_current ();

  for (int i = 2; i < FDT_COUNT_LIMIT; i++)
  {
    if (cur->fdt[i])
      continue;
    cur->next_fd = i + 1;
    cur->fdt[i] = f;
    return i;
  }
  
  file_close (f);

  return -1;
}

int 
filesize (int fd)
{
  struct file *f = thread_current ()->fdt[fd];

  if (f == NULL)
    return -1;
  
  return file_length (f);
}

int 
read (int fd, void *buffer, unsigned length)
{
  valid_address (buffer);

  int count = 0;
  unsigned char *buf = buffer; //protect negative unsigned
  
  char key;
  if (fd == 0)
  {
    for (; count < length; count++)
    {
      key = input_getc ();
      *buf = key;
      buf++;
      if (key == '\0')
        break;
    }
  }
  else if (fd == 1)
    return -1;
  else
  {
    struct file *f = thread_current ()->fdt[fd];

    if (f == NULL)
      return -1;
    
    lock_acquire (&filesys_lock);
    count = file_read (f, buffer, length);
    lock_release (&filesys_lock);
  }
  
  return count;
}

int
write (int fd, const void *buffer, unsigned length)
{
  valid_address (buffer);

  int count = 0;

  if (fd == 0)
    return -1;
  else if (fd == 1)
  {
    putbuf (buffer, length);
    return length;
  }
  else
  {
    struct file *f = thread_current ()->fdt[fd];
    if (f == NULL)
      return -1;
    lock_acquire (&filesys_lock);
    count = file_write (f, buffer, length);
    lock_release (&filesys_lock);
  }

  return count;
}

void 
seek (int fd, unsigned position)
{
  struct file *f = thread_current ()->fdt[fd];
  if (f == NULL)
    return
  file_seek (f, position);
}

unsigned 
tell (int fd)
{
  struct file *f = thread_current ()->fdt[fd];
  if (f == NULL)
    return -1;
  return file_tell (f);
}

void
close (int fd)
{
  struct file *f = thread_current ()->fdt[fd];
  file_close (f);
}
