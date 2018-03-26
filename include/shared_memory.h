#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define PROC_CTRL_TBL_SZE 18

#include <stdbool.h>
#include "clock.h"

enum Status { 
    RUNNING,
    READY,
    TERMINATED, 
    BLOCKED,
};

struct process_ctrl_block {
    int pid;
    enum Status status;
    bool is_realtime;
    unsigned int time_quantum;
    struct clock cpu_time_used;     // total CPU time used
    struct clock sys_time_used;     // total system time used
    unsigned long last_run;          // length of time of last run in nanoseconds
    struct clock time_blocked;      // total time process is blocked for
    struct clock time_unblocked;    // time when the process is unblocked
    struct clock time_scheduled;    // time when process is scheduled
    struct clock time_finished;     // time when process terminates
};

struct process_ctrl_table {
    struct process_ctrl_block pcbs[PROC_CTRL_TBL_SZE];
};

int get_shared_memory();
void* attach_to_shared_memory(int shmemid, unsigned int readonly);
void cleanup_shared_memory(int shmemid, void* p);
void detach_from_shared_memory(void* p);
void deallocate_shared_memory(int shmemid);

#endif
