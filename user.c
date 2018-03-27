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
unsigned int get_random_pct();
void add_signal_handlers();
void handle_sigterm(int sig);
struct clock get_time_to_request_release_rsc(struct clock sysclock);
unsigned int get_nanosecs_to_request_release();

#define ONE_HUNDRED_MILLION 100000000 // 100ms in nanoseconds

const unsigned int CHANCE_TERMINATE = 4;
const unsigned int MAX_CLAIMS = 3; 

int main (int argc, char *argv[]) {
    add_signal_handlers();
    srand(time(NULL) ^ getpid());
    setlocale(LC_NUMERIC, "");      // For comma separated integers in printf
    unsigned int nanosecs;

    // Get shared memory IDs
    int sysclock_id = atoi(argv[SYSCLOCK_ID_IDX]);
    int rsc_tbl_id = atoi(argv[RSC_TBL_ID_IDX]);
    int rsc_msg_box_id = atoi(argv[RSC_MSGBX_ID_IDX]);
    int pid = atoi(argv[PID_IDX]);
    
    // Attach to shared memory
    struct clock* sysclock = attach_to_shared_memory(sysclock_id, 1);
    struct resource_table* rsc_tbl = attach_to_shared_memory(rsc_tbl_id, 0);
    
    struct msgbuf rsc_msg_box;

    struct clock time_to_request_release = get_time_to_request_release_rsc(*sysclock);

    while(1) {
        // Blocking receive - wait until scheduled
        receive_msg(rsc_msg_box_id, &rsc_msg_box, pid);
        // Received message from OSS telling me to run
        
        if (will_terminate()) {

            break;
        }
        
        // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
        send_msg(rsc_msg_box_id, &rsc_msg_box, (pid + PROC_CTRL_TBL_SZE)); 
    }

    // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
    send_msg(rsc_msg_box_id, &rsc_msg_box, (pid + PROC_CTRL_TBL_SZE)); 

    return 0;  
}

bool will_terminate() {
    return event_occured(CHANCE_TERMINATE);
}

unsigned int get_random_pct() {
    return (rand() % 99) + 1;
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

struct clock get_time_to_request_release_rsc(struct clock sysclock) {
    unsigned int nanoseconds = get_nanosecs_to_request_release();
    struct clock time_to_request_release = sysclock;
    increment_clock(&time_to_request_release, nanoseconds)
    return time_to_request_release;
}

unsigned int get_nanosecs_to_request_release() {
    unsigned int lower_bound = 10000000;
    return (rand() % (ONE_HUNDRED_MILLION - lower_bound)) + lower_bound;
}