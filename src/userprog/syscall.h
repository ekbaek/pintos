#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "threads/synch.h"
#include "lib/user/syscall.h"

struct lock filesys_lock;
void syscall_init (void);

// syscall func
void halt (void);
void exit (int status);
int fork (const char *thread_name, struct intr_frame *f);
pid_t exec (const char *cmd_line);
int wait (int pid);

#endif /* userprog/syscall.h */
