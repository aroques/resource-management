#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/time.h>
#include <locale.h>
#include <signal.h>

#include "global_constants.h"
#include "helpers.h"
#include "message_queue.h"
#include "shared_memory.h"
#include "clock.h"

bool will_terminate();
bool use_entire_timeslice();
unsigned int get_random_pct();
struct clock get_event_wait_time();
void add_signal_handlers();
void handle_sigterm(int sig);


const unsigned int CHANCE_TERMINATE = 4;
const unsigned int MAX_CLAIMS = 3; 

int main (int argc, char *argv[]) {
    add_signal_handlers();
    srand(time(NULL) ^ getpid());
    setlocale(LC_NUMERIC, "");      // For comma separated integers in printf
    unsigned int nanosecs;

    // Get shared memory IDs
    int sysclock_id = atoi(argv[SYSCLOCK_ID_IDX]);
    int proc_ctrl_tbl_id = atoi(argv[PCT_ID_IDX]);
    int pid = atoi(argv[PID_IDX]);
    int scheduler_id = atoi(argv[SCHEDULER_IDX]);
    
    // Attach to shared memory
    struct clock* sysclock = attach_to_shared_memory(sysclock_id, 1);
    struct process_ctrl_table* pct = attach_to_shared_memory(proc_ctrl_tbl_id, 0);

    struct process_ctrl_block* pcb = &pct->pcbs[pid];
    
    struct msgbuf scheduler;
    while(1) {
        // Blocking receive - wait until scheduled
        receive_msg(scheduler_id, &scheduler, pid);
        // Received message from OSS telling me to run
        pcb->status = RUNNING;
        
        if (will_terminate()) {
            // Run for some random pct of time quantum
            nanosecs = pcb->time_quantum / get_random_pct();
            pcb->last_run = nanosecs;
            increment_clock(&pcb->cpu_time_used, nanosecs);

            break;
        }
        
        // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
        send_msg(scheduler_id, &scheduler, (pid + PROC_CTRL_TBL_SZE)); 
    }

    pcb->status = TERMINATED;

    pcb->time_finished.seconds = sysclock->seconds;
    pcb->time_finished.nanoseconds = sysclock->nanoseconds;

    pcb->sys_time_used = subtract_clocks(pcb->time_finished, pcb->time_scheduled);

    // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
    send_msg(scheduler_id, &scheduler, (pid + PROC_CTRL_TBL_SZE)); 

    return 0;  
}

bool will_terminate() {
    return event_occured(CHANCE_TERMINATE);
}

unsigned int get_random_pct() {
    return (rand() % 99) + 1;
}

struct clock get_event_wait_time() {
    struct clock event_wait_time;
    event_wait_time.seconds = rand() % 6;
    event_wait_time.nanoseconds = rand() % 1001;
    return event_wait_time;
}

void add_signal_handlers() {
    struct sigaction act;
    act.sa_handler = handle_sigterm; // Signal handler
    sigemptyset(&act.sa_mask);      // No other signals should be blocked
    act.sa_flags = 0;               // 0 so do not modify behavior
    if (sigaction(SIGTERM, &act, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void handle_sigterm(int sig) {
    //printf("USER %d: Caught SIGTERM %d\n", getpid(), sig);
    _exit(0);
}