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
bool has_resource(int pid, struct resource_table* rsc_tbl);
bool will_release_resource();
unsigned int get_resource_to_release(int pid, struct resource_table* rsc_tbl);
void request_a_resource(int rsc_msg_box_id, int pid);
void release_a_resource(int rsc_msg_box_id, int pid, struct resource_table* rsc_tbl);
unsigned int* get_allocated_resources(int pid, struct resource_table* rsc_tbl);
unsigned int get_number_of_allocated_rsc_classes(int pid, struct resource_table* rsc_tbl);
void send_termination_notification(int rsc_msg_box_id, int pid);

#define ONE_HUNDRED_MILLION 100000000 // 100ms in nanoseconds
#define TEN_MILLION 10000000 // 10ms in nanoseconds

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

    struct clock time_to_request_release = get_time_to_request_release_rsc(*sysclock);

    while(1) {
        if (compare_clocks(*sysclock, time_to_request_release) < 0) {
            continue;
        }
        // Time to request/release a resource 
        if (!has_resource(pid, rsc_tbl)) {
            request_a_resource(rsc_msg_box_id, pid);
            // if (will_terminate()) {
            //     send_termination_notification(rsc_msg_box_id, pid);
            //     break;
            // }
        }
        else {
            // Determine if we are going to request or release a resource
            if (will_release_resource()) {
                release_a_resource(rsc_msg_box_id, pid, rsc_tbl);
            }
            else {
                request_a_resource(rsc_msg_box_id, pid);
                // if (will_terminate()) {
                //     send_termination_notification(rsc_msg_box_id, pid);
                //     break;
                // }
            }
        }
        // Get new time to request/release a resouce
        time_to_request_release = get_time_to_request_release_rsc(*sysclock);
    } 

    return 0;  
}

void send_termination_notification(int rsc_msg_box_id, int pid) {
    struct msgbuf rsc_msg_box;
    sprintf(rsc_msg_box.mtext, "TERM");
    send_msg(rsc_msg_box_id, &rsc_msg_box, pid); 
}

void release_a_resource(int rsc_msg_box_id, int pid, struct resource_table* rsc_tbl) {
    struct msgbuf rsc_msg_box;
    unsigned int resource_to_release = get_resource_to_release(pid, rsc_tbl);
    sprintf(rsc_msg_box.mtext, "RLS%d", resource_to_release);
    send_msg(rsc_msg_box_id, &rsc_msg_box, pid);
    return;
}

void request_a_resource(int rsc_msg_box_id, int pid) {
    struct msgbuf rsc_msg_box;
    create_msg_that_contains_rsc(rsc_msg_box.mtext);
    send_msg(rsc_msg_box_id, &rsc_msg_box, pid);
    // Blocking receive - wait until granted a resource
    receive_msg(rsc_msg_box_id, &rsc_msg_box, pid);
    // Granted a resource
    return;
}

unsigned int get_resource_to_release(int pid, struct resource_table* rsc_tbl) {
    unsigned int* allocated_resources = get_allocated_resources(pid, rsc_tbl);
    size_t size = sizeof(allocated_resources) / sizeof(allocated_resources[0]);
    unsigned int random_idx = rand() % size;
    unsigned int resource_to_release = allocated_resources[random_idx];
    free(allocated_resources);
    return resource_to_release;
}

bool will_release_resource() {
    return event_occured(CHANCE_RELEASE);
}

bool will_terminate() {
    return event_occured(CHANCE_TERMINATE);
}

void create_msg_that_contains_rsc(char* mtext) {
    // NEED TO ADD LOGIC SO THAT WE ONLY REQUEST A RESOURCE IF IT DOES NOT
    // VIOLATE OUR MAX CLAIMS
    unsigned int resource_to_request = get_random_resource();
    sprintf(mtext, "REQ%d", resource_to_request);
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
    unsigned int lower_bound = 1000000;
    return (rand() % (TEN_MILLION - lower_bound)) + lower_bound;
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

unsigned int* get_allocated_resources(int pid, struct resource_table* rsc_tbl) {
    // Returns an array of all resource classes that are currently allocated
    unsigned int num_resources, num_resource_classes = 0;
    unsigned int i, j;

    num_resource_classes = get_number_of_allocated_rsc_classes(pid, rsc_tbl);

    unsigned int* allocated_resources = malloc(sizeof(unsigned int) * num_resource_classes);
    j = 0;
    for (i = 0; i < NUM_RSC_CLS; i++) {
        num_resources = rsc_tbl->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            allocated_resources[j++] = i;
        }
    }
    return allocated_resources;
}

unsigned int get_number_of_allocated_rsc_classes(int pid, struct resource_table* rsc_tbl) {
    unsigned int num_resources, num_resource_classes = 0;
    unsigned int i;
    for (i = 0; i < NUM_RSC_CLS; i++) {
        num_resources = rsc_tbl->rsc_descs[i].allocated[pid];
        if (num_resources > 0) {
            num_resource_classes++;
        }
    }
    return num_resource_classes;
}