#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
//추가함
#include "threads/palloc.h"


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
			f->R.rax = process_wait(f->edi); //process wait 수정하러 가야함
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
  t->exit_status = status;
  printf("%s: exit(%d)\n", t->name, status);
  thread_exit();
}

int
fork (const char *thread_name, struct intr_frame *f)
{
	return process_fork(thread_name, f);
}

int
exec (const char *file_name) 
{
  check_verify(file_name);

  char *fn_copy;
  fn_copy = palloc_get_page(0);  
  if (fn_copy == NULL)
    exit(-1);
  strlcpy (fn_copy, file_name, strlen(file_name) + 1);

  if (process_execute(fn_copy) == -1) 
    exit(-1);
}




