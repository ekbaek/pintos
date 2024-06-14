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
  // valid_address (file);
  // struct file *f = filesys_open (file);

  // if (f == NULL)
  //   return -1;
  
  // struct thread *cur = thread_current ();

  // for (int i = 2; i < FDT_COUNT_LIMIT; i++)
  // {
  //   if (cur->fdt[i])
  //     continue;
  //   cur->next_fd = i + 1;
  //   cur->fdt[i] = f;
  //   return i;
  // }
  
  // file_close (f);

  // return -1;
  if (file == NULL)
    exit(-1);
  valid_address (file);
  // lock_acquire (&file_lock);
  struct file *return_file = filesys_open (file);
  if (return_file == NULL)
    return -1;
  else
  {
    for (int i=3; i<128; i++)
    {
      if (getfile(i) == NULL)
      {
        if (strcmp (thread_current()->name, file) == false)
          file_deny_write (return_file);

        thread_current()->fdt[i] = return_file;
//printf("  >> filesys_open(file) success, return %d, idx of fd", i);
        // lock_release (&file_lock);
        return i;
      }
    }
//printf("  >> filesys_open(file) failed ; thread's fd is full, return -1\n");
  }
  // lock_release (&file_lock);
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

  // int count = 0;
  // unsigned char *buf = buffer; //protect negative unsigned
  
  // char key;
  // if (fd == 0)
  // {
  //   for (; count < length; count++)
  //   {
  //     key = input_getc ();
  //     *buf = key;
  //     buf++;
  //     if (key == '\0')
  //       break;
  //   }
  // }
  // else if (fd == 1)
  //   return -1;
  // else
  // {
  //   struct file *f = thread_current ()->fdt[fd];

  //   if (f == NULL)
  //     return -1;
    
  //   lock_acquire (&filesys_lock);
  //   count = file_read (f, buffer, length);
  //   lock_release (&filesys_lock);
  // }
  
  // return count;
  if (fd == 0)
  {
    /* input_getc() 를 이용해 키보드 입력을 버퍼에 넣는다. 그리고 입력된 사이즈(bytes)를 리턴한다. */
    int i;
    for (i=0; i<length; i++)
    {
      if ( ( (char *)buffer)[i] == '\0')
        break;
    }
    // lock_release (&file_lock);
    return i;
  }
  else
  {
    struct file *f = getfile (fd);
    if (f == NULL)
      exit(-1);
    else
    {
      // lock_release (&file_lock);
      return file_read (f, buffer, length);
    }
  }
}

int
write (int fd, const void *buffer, unsigned length)
{
  // valid_address (buffer);

  // int count = 0;

  // if (fd == 0)
  //   return -1;
  // else if (fd == 1)
  // {
  //   putbuf (buffer, length);
  //   return length;
  // }
  // else
  // {
  //   struct file *f = thread_current ()->fdt[fd];
  //   if (f == NULL)
  //     return -1;
  //   lock_acquire (&filesys_lock);
  //   count = file_write (f, buffer, length);
  //   lock_release (&filesys_lock);
  // }
  //   return count;
  valid_address (buffer);
  // lock_acquire (&file_lock);
  if (fd == 1)
  {
    /* putbuf() 함수를 이용하여 버퍼의 내용을 콘솔에 입력한다. 이 때에는 필요한 사이즈만큼 반복문을 돌아야 한다. */
    putbuf (buffer, length);
    return length;
  }
  else
  {
    struct file *f = getfile (fd);
    if (f == NULL)
    {
      // lock_release (&file_lock);
      exit(-1);
    }
    if (f)
    {
      file_deny_write (f);
    }
    // lock_release (&file_lock);
    return file_write (f, buffer, length);
  }
}

struct file
*getfile (int fd)
{
  return (thread_current()->fdt[fd]);
}

void 
seek (int fd, unsigned position)
{
  // struct file *f = thread_current ()->fdt[fd];
  // if (f == NULL)
  //   return 
  // file_seek (f, position);
  struct file *f = getfile (fd);
  if (f == NULL)
    exit(-1);
  else
    return file_seek (f, position);
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
  // struct file *f = thread_current ()->fdt[fd];
  // file_close (f);
  struct file *f = getfile (fd);
  if (f == NULL)
    exit(-1);
  else
  {
    f = NULL;
    file_close (f);
  }
}


// part2

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

pid_t
exec (const char *cmd_line)
{
  valid_address(cmd_line);

  char *fn_copy;
  fn_copy = palloc_get_page(PAL_ZERO);  
  if (fn_copy == NULL)
    exit(-1);

  strlcpy (fn_copy, cmd_line, strlen(cmd_line) + 1);

  if (process_execute(fn_copy) == -1) 
    exit(-1);
  
  NOT_REACHED();
  return 0;
}

int 
wait (pid_t pid)
{
  return process_wait (pid);
}