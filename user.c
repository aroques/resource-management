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
void create_msg_that_contains_rsc(char* mtext);
unsigned int get_random_resource();
bool has_resource(struct resource_table* rsc_tbl);
bool will_release_resource();
unsigned int get_resouce_to_release(int pid, struct resource_table* rsc_tbl);

#define ONE_HUNDRED_MILLION 100000000 // 100ms in nanoseconds

const unsigned int CHANCE_TERMINATE = 10;
const unsigned int CHANCE_RELEASE = 50;
const unsigned int MAX_CLAIMS = 3; 

int main (int argc, char *argv[]) {
    add_signal_handlers();
    srand(time(NULL) ^ getpid());
    setlocale(LC_NUMERIC, "");      // For comma separated integers in printf

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
    unsigned int resource_to_request, resource_to_release;

   while(1) {
        if (compare_clocks(*sysclock, time_to_request_release) < 0) {
            continue;
        }
        // Time to request/release a resource 
        if (!has_resource) {
            // Request a resource
            create_msg_that_contains_rsc(rsc_msg_box.mtext);
            send_msg(rsc_msg_box_id, &rsc_msg_box, pid); // Mtype = pid
            // Blocking receive - wait until granted a resource
            receive_msg(rsc_msg_box_id, &rsc_msg_box, pid + MAX_PROC_CNT); // Mtype = pid + MAX_PROC_COUNT
            // Granted a resource
            if (will_terminate()) {
                break;
            }
        }
        else {
            // Determine if we are going to request or release a resource
            if (will_release_resource()) {
                // Release a resource
                resource_to_release = get_resouce_to_release(pid, rsc_tbl);
                sprintf(rsc_msg_box.mtext, "%d", resource_to_release);
                send_msg(rsc_msg_box_id, &rsc_msg_box, pid); // Mtype = pid
            }
            else {
                // Request a resource
                create_msg_that_contains_rsc(rsc_msg_box.mtext);
                send_msg(rsc_msg_box_id, &rsc_msg_box, pid); // Mtype = pid
                // Blocking receive - wait until granted a resource
                receive_msg(rsc_msg_box_id, &rsc_msg_box, pid + MAX_PROC_CNT); // Mtype = pid + MAX_PROC_COUNT
                // Granted a resource
                if (will_terminate()) {
                    break;
                }
            }
        }
        // Get new time to request/release a resouce
        time_to_request_release = get_time_to_request_release_rsc(*sysclock);
    } 
    printf("user: hello world!\n");
    return 0;  
}

unsigned int get_resouce_to_release(int pid, struct resource_table* rsc_tbl) {
    int* allocated_resources = get_allocated_resources(pid, rsc_tbl);
    size_t size = sizeof(allocated_resources) / sizeof(allocated_resources[0]);
    unsigned int random_idx = rand() % size;
    unsigned int resource_to_release = allocated_resources[random_idx];
    return resource_to_release;
}

bool will_release_resource() {
    return event_occured(CHANCE_RELEASE);
}

bool will_terminate() {
    return event_occured(CHANCE_TERMINATE);
}

void create_msg_that_contains_rsc(char* mtext) {
    unsigned int resource_to_request = get_random_resource();
    sprintf(mtext, "%d", resource_to_request);
}

unsigned int get_random_resource() {
    return rand() % NUM_RSC_CLS;
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
    increment_clock(&time_to_request_release, nanoseconds);
    return time_to_request_release;
}

unsigned int get_nanosecs_to_request_release() {
    unsigned int lower_bound = 10000000;
    return (rand() % (ONE_HUNDRED_MILLION - lower_bound)) + lower_bound;
}

bool has_resource(int pid, struct resource_table* rsc_tbl) {
    unsigned int i;
    unsigned int num_resources = 0;
    for (i = 0; i < NUM_RSC_CLS; i++) {
        num_resources = rsc_tbl->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            return 1;
        }
    }
    return 0;
}

int* get_allocated_resources(int pid, struct resource_table* rsc_tbl) {
    // Returns an array of all resource classes that are currently allocated
    unsigned int num_resource_cls = 0;
    for (i = 0; i < NUM_RSC_CLS; i++) {
        num_resources = rsc_tbl->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            num_resource_cls++;
        }
    }
    unsigned int allocated_resources[num_resource_cls];
    unsigned int j = 0;
    for (i = 0; i < NUM_RSC_CLS; i++) {
        num_resources = rsc_tbl->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            allocated_resources[j++] = i;
        }
    }
    return allocated_resources;
}