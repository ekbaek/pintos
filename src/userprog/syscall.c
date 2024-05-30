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

  // copy arguments on the user stack to the kernel
  // save return value of system call at eax register
	switch (f->R.rax)
	{
	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		exit(f->R.rdi);
		break;
	case SYS_EXEC:
    f->R.rax = fork(f->R.rdi, f);
		if (exec(f->R.rdi) == -1)
      exit(-1);
		break;
	case SYS_WAIT:
		f->R.rax = process_wait(f->R.rdi);
		break;
	case SYS_CREATE:
		f->R.rax = create(f->R.rdi, f->R.rsi);
		break;
	case SYS_REMOVE:
		f->R.rax = remove(f->R.rdi);
		break;
	case SYS_OPEN:
		f->R.rax = open(f->R.rdi);
		break;
	case SYS_FILESIZE:
		f->R.rax = filesize(f->R.rdi);
		break;
	case SYS_READ:
		f->R.rax = read(f->R.rdi, f->R.rsi, f->R.rdx);
		break;
	case SYS_WRITE:
		f->R.rax = write(f->R.rdi, f->R.rsi, f->R.rdx);
		break;
	case SYS_SEEK:
		seek(f->R.rdi, f->R.rsi);
		break;
	case SYS_TELL:
		f->R.rax = tell(f->R.rdi);
		break;
	case SYS_CLOSE:
		close(f->R.rdi);
		break;
	default:
		exit(-1);
		break;
	}  
}

// check validation of the pointers in parameter list
void
check_validation (struct intr_frame *f)
{

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

pid_t
exec (const char *cmd_line) 
{
  //create child
  struct intr_frame *f;
  process_fork(thread_name(), f);
  // exec program
  process_execute(cmd_line);
}

int
wait (pid_t pid)
{
  process_wait(pid);
}

// create file
bool
create (const char *file_name, off_t initial_size)
{
  check_validation(file_name);
  return filesys_create(file_name, initial_size);
}

fork

// delete a fiel
remove

open

filesize 

read 

write 

seek 

tell 

close 

