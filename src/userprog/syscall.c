#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h" 
#include "threads/palloc.h"
#include "lib/user/syscall.h"
#include "kernel/stdio.h"

static void syscall_handler (struct intr_frame *);
struct file *getfile (int fd);
void valid_address(const uint64_t *cur_addr);


void valid_address(const uint64_t *cur_addr)
{
	if (!(is_user_vaddr(cur_addr)))
	{
		exit(-1);
	}
}

void
syscall_init (void) 
{
  //lock_init (&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  void *sp = f->esp;

  switch (*(uint32_t *)sp)
  {
    case SYS_HALT:                   // args number: 0
      halt ();
      break;

    case SYS_EXIT:                   // args number: 1
      valid_address (sp + 4);
      exit( *(uint32_t *)(sp + 4) );
      break;

    case SYS_EXEC:                   // args number: 1
      valid_address (sp + 4);
      f->eax = exec ( (const char *)*(uint32_t *)(sp + 4) );
      break;

    case SYS_WAIT:                   // args number: 1
      valid_address (sp + 4);
      f->eax = wait ( (pid_t *)*(uint32_t *)(sp + 4) );
      break;

    case SYS_CREATE:                 // args number: 2
      valid_address (sp + 4);
      f->eax = create ( (const char *)*(uint32_t *)(sp + 4),  (const char *)*(uint32_t *)(sp + 8) );
      break;

    case SYS_REMOVE:                 // args number: 1
      valid_address (sp + 4);
      f->eax = remove ( (const char *)*(uint32_t *)(sp + 4) );
      break;

    case SYS_OPEN:                   // args number: 1
      valid_address (sp + 4);
      f->eax = open ( (const char *)*(uint32_t *)(sp + 4) );
      break;

    case SYS_FILESIZE:               // args number: 1
      valid_address (sp + 4);
      f->eax = filesize ( (int)*(uint32_t *)(sp + 4) );
      break;

    case SYS_READ:                   // args number: 3
      valid_address (sp + 4);
      f->eax = read ( (int)*(uint32_t *)(sp + 4), (void *)*(uint32_t *)(sp + 8), (unsigned)*((uint32_t *)(sp + 12)) );
      break;

    case SYS_WRITE:                  // args number: 3
      valid_address (sp + 4);
      f->eax = write( (int)*(uint32_t *)(sp + 4), (void *)*(uint32_t *)(sp + 8), (unsigned)*((uint32_t *)(sp + 12)) );
      break;

    case SYS_SEEK:                   // args number: 2
      valid_address (sp + 4);
      seek ( (int)*(uint32_t *)(sp + 4), (unsigned)*((uint32_t *)(sp + 8)) );
      break;

    case SYS_TELL:                   // args number: 1
      valid_address (sp + 4);
      f->eax = tell ( (int)*(uint32_t *)(sp + 4) );
      break;

    case SYS_CLOSE:                  // args number: 1
      valid_address (sp + 4);
      close ( (int)*(uint32_t *)(sp + 4) );
      break;
  }
}

bool 
create (const char *file, unsigned initial_size)
{
  valid_address (file);
  if (file == NULL)
    exit(-1);
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
  if (file == NULL)
    exit(-1);
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
  if (file == NULL)
    exit(-1);
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
    
    //lock_acquire (&filesys_lock);
    count = file_read (f, buffer, length);
    //lock_release (&filesys_lock);
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
    //lock_acquire (&filesys_lock);
    count = file_write (f, buffer, length);
    //lock_release (&filesys_lock);
  }
    return count;
}

void 
seek (int fd, unsigned position)
{
  struct file *f = thread_current ()->fdt[fd];
  if (f == NULL)
    exit(-1);
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
  if (f == NULL)
    exit(-1);
  else
  {
    f = NULL;
    file_close (f);
  }
}

void 
halt (void)
{
  shutdown_power_off ();
}

void
exit (int status)
{ 
  struct thread *t = thread_current();
  t->status_exit = status;
  printf("%s: exit(%d)\n", thread_name(), status);
  
  for (int i=3; i<128; i++) 
  {
    if (thread_current ()->fdt[i] != NULL)
      close(i);
  }
  thread_exit();
}

pid_t exec (const char *cmd_line)
{
  valid_address(cmd_line);

  char *fn_copy;
  fn_copy = palloc_get_page(PAL_ZERO);  
  if (fn_copy == NULL)
    exit(-1);

  strlcpy(fn_copy, cmd_line, strlen(cmd_line) + 1);

  struct thread *cur = thread_current();
  cur->complete_load = 0;

  pid_t pid = process_execute(fn_copy);
  if (pid == -1) 
    exit(-1);

  sema_down(&cur->load_semaphore);

  if (cur->complete_load == -1)
    return -1;
  
  return pid;
}

int 
wait (pid_t pid)
{
  return process_wait (pid);
}