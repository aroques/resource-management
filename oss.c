#include <stdio.h>
#include <sys/wait.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <locale.h>
#include <sys/time.h>
#include <time.h>
#include <sys/queue.h>
#include <math.h>
#include <errno.h>

#include "global_constants.h"
#include "helpers.h"
#include "shared_memory.h"
#include "message_queue.h"
#include "queue.h"
#include "resources.h"
#include "bankers.h"

void wait_for_all_children();
void add_signal_handlers();
void handle_sigint(int sig);
void handle_sigalrm(int sig);
void cleanup_and_exit();
void fork_child(char** execv_arr, unsigned int pid);
struct clock get_time_to_fork_new_proc(struct clock sysclock);
unsigned int get_nanoseconds();
unsigned int get_available_pid();
int get_rsc_from_msg(char* mtext);

// Globals used in signal handler
int simulated_clock_id, rsc_tbl_id, rsc_msg_box_id;
struct clock* sysclock;                                 
struct resource_table* rsc_tbl;
int cleaning_up = 0;
pid_t* childpids;
FILE* fp;

int main (int argc, char* argv[]) {
    /*
     *  Setup program before entering main loop
     */
    const unsigned int TOTAL_RUNTIME = 2;       // Max seconds oss should run for

    set_timer(MAX_RUNTIME);                     // Set timer that triggers SIGALRM
    add_signal_handlers();          
    setlocale(LC_NUMERIC, "");                  // For comma separated integers in printf
    srand(time(NULL) ^ getpid());

    unsigned int i, pid;
    char buffer[255];                           // Used to hold output that will be printed and written to log file
    unsigned int elapsed_seconds = 0;           // Holds total real-time seconds the program has run
    struct timeval tv_start, tv_stop;           // Used to calculated real elapsed time
    gettimeofday(&tv_start, NULL);
    //struct Queue blocked;                       // Holds processes that are blocked on resources
    unsigned int proc_cnt = 0;                  // Holds total number of active child processes

    struct clock time_to_fork = get_clock();    // Holds time to schedule new process

    // Setup execv array to pass initial data to children processes
    char* execv_arr[EXECV_SIZE];                
    execv_arr[0] = "./user";
    execv_arr[EXECV_SIZE - 1] = NULL;
    
    /*
     *  Setup shared memory
     */
    // Shared logical clock
    simulated_clock_id = get_shared_memory();
    sysclock = (struct clock*) attach_to_shared_memory(simulated_clock_id, 0);
    reset_clock(sysclock);
    // Shared Resource Table 
    rsc_tbl_id = get_shared_memory();
    rsc_tbl = (struct resource_table*) attach_to_shared_memory(rsc_tbl_id, 0);
    allocate_rsc_tbl(rsc_tbl);
    // Shared resource message box for user processes to request/release resources 
    rsc_msg_box_id = get_message_queue();
    struct msgbuf rsc_msg_box;
    char notification_type[10];

    // Holds all childpids
    childpids = malloc(sizeof(pid_t) * (MAX_PROC_CNT + 1));
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        childpids[i] = 0;
    }

    // Open log file for writing
    if ((fp = fopen("./oss.log", "w")) == NULL) {
        perror("fopen");
        exit(1);
    }

    // Get a time to fork first process at
    time_to_fork = get_time_to_fork_new_proc(*sysclock);
    
    // Increment current time so it is time to fork a user process
    *sysclock = time_to_fork;

    /*
     *  Main loop
     */
    print_available_rsc_tbl(rsc_tbl, fp);
    while ( elapsed_seconds < TOTAL_RUNTIME ) {
        // Check if it is time to fork a new user process
        if (compare_clocks(*sysclock, time_to_fork) >= 0 && proc_cnt < MAX_PROC_CNT) {
            // Fork a new process
            pid = get_available_pid();
            rsc_tbl->max_claims[pid] = get_max_resource_claims();
            fork_child(execv_arr, pid);
            proc_cnt++;

            sprintf(buffer, "OSS: Generating P%d at time %ld:%'ld\n",
                pid, sysclock->seconds, sysclock->nanoseconds);
            print_and_write(buffer, fp);
    
            time_to_fork = get_time_to_fork_new_proc(*sysclock);
        }

        // Check for any messages
        for (i = 1; i <= MAX_PROC_CNT; i++) {
            receive_msg_no_wait(rsc_msg_box_id, &rsc_msg_box, i);
            if (strlen(rsc_msg_box.mtext) == 1) {
                continue;
            }

            strncpy(notification_type, rsc_msg_box.mtext, 3);

            int resource = get_rsc_from_msg(rsc_msg_box.mtext);

            if (strcmp(notification_type, "REQ") == 0) {
                // Process i is requesting a resource
                sprintf(buffer, "OSS: P%d requesting R%d at time %ld:%'ld\n",
                    i, resource+1, sysclock->seconds, sysclock->nanoseconds);
                print_and_write(buffer, fp);

                bool rsc_granted = 0;
                bool rsc_is_available = resource_is_available(rsc_tbl, resource);
                char reason[50] = "resource is unavailable";
                
                if (rsc_is_available) {
                    // Resource is available so run bankers algorithm to check if we grant
                    // safely grant this request
                    rsc_granted = bankers_algorithm(rsc_tbl, i, resource);
                    sprintf(reason, "granting this resource would lead to an unsafe state");
                }
                
                if (rsc_granted) {
                    // Resource granted
                    sprintf(buffer, "OSS: Granting P%d R%d at time %ld:%'ld\n",
                        i, resource+1, sysclock->seconds, sysclock->nanoseconds);
                    print_and_write(buffer, fp);

                    // Update program state
                    rsc_tbl->rsc_descs[resource].allocated[i]++;

                    // Send message back to user program to let it know that it's request was granted
                    send_msg(rsc_msg_box_id, &rsc_msg_box, i);
                }
                else {
                    // Resource was not granted
                    sprintf(buffer, "OSS: Blocking P%d for requesting R%d at time %ld:%'ld because %s\n",
                        i, resource+1, sysclock->seconds, sysclock->nanoseconds, reason);
                    print_and_write(buffer, fp);

                    // TBD: Put in blocked queue
                    // TBD: blocked queue will need struct with resource and pid
                    // Let the process stay waiting on a message from OSS
                }

            }
            else if (strcmp(notification_type, "RLS") == 0) {
                // Process i is releasing a resource
                sprintf(buffer, "OSS: P%d released R%d at time %ld:%'ld\n",
                    i, resource+1, sysclock->seconds, sysclock->nanoseconds);
                print_and_write(buffer, fp);
                
                // Update program state
                rsc_tbl->rsc_descs[resource].allocated[i]--;

                // TBD: Check to see if we can unblock any processes
            }
            else {
                // Process i terminated
                sprintf(buffer, "OSS: P%d terminated at time %ld:%'ld\n",
                    i, sysclock->seconds, sysclock->nanoseconds);
                print_and_write(buffer, fp);

                // Update program state
                childpids[i] = 0;
                proc_cnt--;
                release_resources(rsc_tbl, i); // Updates resource table
                
                // TBD: Check to see if we can unblock any processes

            }
            sprintf(buffer, "\n");
            print_and_write(buffer, fp);
        }

        increment_clock(sysclock, get_nanoseconds());

        // Calculate total elapsed real-time seconds
        gettimeofday(&tv_stop, NULL);
        elapsed_seconds = tv_stop.tv_sec - tv_start.tv_sec;
    }

    // Print information before exiting
    sprintf(buffer, "OSS: Exiting because %d seconds have been passed\n", TOTAL_RUNTIME);
    print_and_write(buffer, fp);

    print_allocated_rsc_tbl(rsc_tbl, fp);
    print_available_rsc_tbl(rsc_tbl, fp);

    cleanup_and_exit();

    return 0;
}

void fork_child(char** execv_arr, unsigned int pid) {
    if ((childpids[pid] = fork()) == 0) {
        // Child so...
        char clock_id[10];
        char rtbl_id[10];
        char rmsgbox_id[10];
        char p_id[5];
        
        sprintf(clock_id, "%d", simulated_clock_id);
        sprintf(rtbl_id, "%d", rsc_tbl_id);
        sprintf(rmsgbox_id, "%d", rsc_msg_box_id);
        sprintf(p_id, "%d", pid);
        
        execv_arr[SYSCLOCK_ID_IDX] = clock_id;
        execv_arr[RSC_TBL_ID_IDX] = rtbl_id;
        execv_arr[RSC_MSGBX_ID_IDX] = rmsgbox_id;
        execv_arr[PID_IDX] = p_id;

        execvp(execv_arr[0], execv_arr);

        perror("Child failed to execvp the command!");
        exit(1);
    }

    if (childpids[pid] == -1) {
        perror("Child failed to fork!\n");
        exit(1);
    }
}

void wait_for_all_children() {
    pid_t pid;
    printf("OSS: Waiting for all children to exit\n");
    fprintf(fp, "OSS: Waiting for all children to exit\n");
    
    while ((pid = wait(NULL))) {
        if (pid < 0) {
            if (errno == ECHILD) {
                perror("wait");
                break;
            }
        }
    }
}

void terminate_children() {
    printf("OSS: Sending SIGTERM to all children\n");
    fprintf(fp, "OSS: Sending SIGTERM to all children\n");
    int i;
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        if (childpids[i] == 0) {
            continue;
        }
        if (kill(childpids[i], SIGTERM) < 0) {
            if (errno != ESRCH) {
                // Child process exists and kill failed
                perror("kill");
            }
        }
    }
    free(childpids);
}

void add_signal_handlers() {
    struct sigaction act;
    act.sa_handler = handle_sigint; // Signal handler
    sigemptyset(&act.sa_mask);      // No other signals should be blocked
    act.sa_flags = 0;               // 0 so do not modify behavior
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    act.sa_handler = handle_sigalrm; // Signal handler
    sigemptyset(&act.sa_mask);       // No other signals should be blocked
    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void handle_sigint(int sig) {
    printf("\nOSS: Caught SIGINT signal %d\n", sig);
    fprintf(fp, "\nOSS: Caught SIGINT signal %d\n", sig);
    if (cleaning_up == 0) {
        cleaning_up = 1;
        cleanup_and_exit();
    }
}

void handle_sigalrm(int sig) {
    printf("\nOSS: Caught SIGALRM signal %d\n", sig);
    fprintf(fp, "\nOSS: Caught SIGALRM signal %d\n", sig);
    if (cleaning_up == 0) {
        cleaning_up = 1;
        cleanup_and_exit();
    }

}

void cleanup_and_exit() {
    terminate_children();
    printf("OSS: Removing message queues and shared memory\n");
    fprintf(fp, "OSS: Removing message queues and shared memory\n");
    remove_message_queue(rsc_msg_box_id);
    wait_for_all_children();
    cleanup_shared_memory(simulated_clock_id, sysclock);
    cleanup_shared_memory(rsc_tbl_id, rsc_tbl);
    fclose(fp);
    exit(0);
}

struct clock get_time_to_fork_new_proc(struct clock sysclock) {
    unsigned int ns_before_next_proc = rand() % MAX_NS_BEFORE_NEW_PROC; 
    increment_clock(&sysclock, ns_before_next_proc);
    return sysclock;
}

unsigned int get_nanoseconds() {
    return (rand() % 30000) + 5001; // 100,000 - 1,000,000 inclusive
}

unsigned int get_available_pid() {
    unsigned int pid, i;
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        if (childpids[i] > 0) {
            continue;
        }
        pid = i;
        break;
    }
    return pid;
}

int get_rsc_from_msg(char* mtext) {
    char resource[2];
    resource[0] = mtext[3];
    resource[1] = mtext[4];
    return atoi(resource);
}