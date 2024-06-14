#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"


// pal zero 사용하려고!
#include "threads/palloc.h" 

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
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
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
			if (exec(f->edi) == -1)
				exit(-1);
			break;
		case SYS_WAIT:
			f->eax = wait(f->edi);
			break;	
    default:
      exit(-1);
      break;
  }
  printf ("system call!\n");
  thread_exit ();
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
  // cur->status를 바꿔야 하나?
  t->status_exit = status;

  // t->name or thread_name()
  printf("%s: exit(%d)\n", thread_name(), status);
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