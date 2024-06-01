#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
//추가함
#include "threads/palloc.h"


static void syscall_handler (struct intr_frame *);
static void check_verify (void *address);
void halt (void);
void exit (int status);
int fork (const char *thread_name, struct intr_frame *f);
pid_t exec (const char *cmd_line);
int wait (int pid);
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
		case SYS_HALT:
			halt();
			break;
		case SYS_EXIT:
			exit(f->edi);
			break;
		case SYS_EXEC:
			f->eax = fork(f->edi, f);
			if (exec(f->edi) == -1)
				exit(-1);
			break;
		case SYS_WAIT:
			f->eax = wait(f->edi);
			break;			
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
	  default:
		  exit(-1);
		  break;
	}  
  //printf ("system call!\n");
  //thread_exit ();
}


// 추가 구현 syscall 함수
void
halt (void) 
{
  shutdown_power_off();
}

void 
exit (int status) 
{
  struct thread *t = thread_current ();
  t->status_to_exit = status;
  printf("%s: exit(%d)\n", t->name, status);
  thread_exit();
}

int
fork (const char *thread_name, struct intr_frame *f)
{
	return process_fork(thread_name, f);
}

pid_t
exec (const char *cmd_line) 
{
  check_verify(cmd_line);

  char *fn_copy;
  fn_copy = palloc_get_page(0);  
  if (fn_copy == NULL)
    exit(-1);
  strlcpy (fn_copy, cmd_line, strlen(cmd_line) + 1);

  if (process_execute(fn_copy) == -1) 
    exit(-1);
}

int
wait (int pid)
{
  return process_wait(pid);
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

int
open (const char *file)
{
  check_verify (file);
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
  check_verify (buffer);

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
    count = filesys_read (f, buffer, length);
    lock_release (&filesys_lock);
  }
  
  return count;
}

int
write (int fd, const void *buffer, unsigned length)
{
  check_verify (buffer);

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