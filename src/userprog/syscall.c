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
