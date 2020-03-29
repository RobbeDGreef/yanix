#ifndef PROC_SCHED_H
#define PROC_SCHED_H

#include <proc/tasking.h>

int getpid();
task_t *get_current_task();

task_t *get_next_task();
int remove_from_ready_list(task_t *task);
void add_task_to_queue(task_t *new_task);
task_t *find_task_by_pid(pid_t pid);

void jump_userspace(reg_t eip, reg_t argc, reg_t argv);
int init_scheduler(task_t *mainloop);

#endif /* PROC_SCHED_H */