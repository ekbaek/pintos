#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H


struct lock filesys_lock;
void syscall_init (void);

// syscall func
void halt (void);
void exit (int status);
pid_t exec (const char *cmd_line);


#endif /* userprog/syscall.h */
