#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H


struct lock filesys_lock;
void syscall_init (void);

// syscall func
void halt (void);
void exit (int status);
int fork (const char *thread_name, struct intr_frame *f);
int exec (const char *file_name);

#endif /* userprog/syscall.h */
