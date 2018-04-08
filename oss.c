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
struct message parse_msg(char* mtext);
void unblock_process_if_possible(int resource, struct msgbuf rsc_msg_box);
unsigned int get_work();
void print_blocked_queue();

unsigned int num_resources_granted = 0;

// Globals used in signal handler
int simulated_clock_id, rsc_tbl_id, rsc_msg_box_id;
struct clock* sysclock;                                 
struct resource_table* rsc_tbl;
int cleaning_up = 0;
pid_t* childpids;
FILE* fp;
struct Queue* blocked;

struct message {
    int pid;
    char txt[10];
    int resource;
};

int main (int argc, char* argv[]) {
    /*
     *  Setup program before entering main loop
     */
    const unsigned int TOTAL_RUNTIME = 2;       // Max seconds oss should run for

    set_timer(MAX_RUNTIME);                     // Set timer that triggers SIGALRM
    add_signal_handlers();          
    setlocale(LC_NUMERIC, "");                  // For comma separated integers in printf
    srand(time(NULL) ^ getpid());

    unsigned int i, pid, num_resources_granted = 0, num_messages = 0;
    char buffer[255];                           // Used to hold output that will be printed and written to log file
    unsigned int elapsed_seconds = 0;           // Holds total real-time seconds the program has run
    struct timeval tv_start, tv_stop;           // Used to calculated real elapsed time
    gettimeofday(&tv_start, NULL);
    blocked = malloc(sizeof(struct Queue) * NUM_RSC_CLS);          // Array of blocked queues (1 for each resource)
    for (i = 0; i < NUM_RSC_CLS; i++) {
        struct Queue bq;
        init_queue(&bq);
        blocked[i] = bq;
    }

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
    print_rsc_summary(rsc_tbl, fp);

    struct msqid_ds msgq_ds;
    msgq_ds.msg_qnum = 0;

    struct message msg;
    int resource;
    bool rsc_granted, rsc_is_available;
    char reason[50];

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

        // Get number of messages
        msgctl(rsc_msg_box_id, IPC_STAT, &msgq_ds);
        num_messages = msgq_ds.msg_qnum;

        // Check for any messages
        while (num_messages > 0) {
            
            receive_msg(rsc_msg_box_id, &rsc_msg_box, 0);
            num_messages--;

            if (strlen(rsc_msg_box.mtext) < 5) {
                // Every once in awhile, the message text is too short to be a real message and will cause segmentation faults if we continue
                // So, just try again
                continue;
            }

            // We received a message from a user process
            msg = parse_msg(rsc_msg_box.mtext);
            resource = msg.resource;
            pid = msg.pid;

            if (strcmp(msg.txt, "REQ") == 0) {
                // Process is requesting a resource
                sprintf(buffer, "OSS: Detected P%d requesting R%d at time %ld:%'ld\n",
                    pid, resource+1, sysclock->seconds, sysclock->nanoseconds);
                print_and_write(buffer, fp);

                rsc_granted = 0;
                rsc_is_available = resource_is_available(rsc_tbl, resource);
                sprintf(reason, "resource is unavailable");
                
                if (rsc_is_available) {
                    // Resource is available so run bankers algorithm to check if we grant
                    // safely grant this request
                    rsc_granted = bankers_algorithm(rsc_tbl, pid, resource);
                    increment_clock(sysclock, get_work());

                    sprintf(reason, "granting this resource would lead to an unsafe state");
                }
                
                if (rsc_granted) {
                    // Resource granted
                    sprintf(buffer, "OSS: Granting P%d R%d at time %ld:%'ld\n",
                        pid, resource+1, sysclock->seconds, sysclock->nanoseconds);
                    print_and_write(buffer, fp);

                    // Update program state
                    rsc_tbl->rsc_descs[resource].allocated[pid]++;
                    num_resources_granted++;
                    
                    if (num_resources_granted % 20 == 0) {
                        // Print table of allocated resources
                        print_allocated_rsc_tbl(rsc_tbl, fp);
                    }

                    // Send message back to user program to let it know that it's request was granted
                    send_msg(rsc_msg_box_id, &rsc_msg_box, pid+MAX_PROC_CNT);
                }
                else {
                    // Resource was not granted
                    sprintf(buffer, "OSS: Blocking P%d for requesting R%d at time %ld:%'ld because %s\n",
                        pid, resource+1, sysclock->seconds, sysclock->nanoseconds, reason);
                    print_and_write(buffer, fp);

                    // Add process to blocked queue
                    enqueue(&blocked[resource], pid);
                }

            }
            else if (strcmp(msg.txt, "RLS") == 0) {
                // Process is releasing a resource
                sprintf(buffer, "OSS: Acknowledging that P%d released R%d at time %ld:%'ld\n",
                    pid, resource+1, sysclock->seconds, sysclock->nanoseconds);
                print_and_write(buffer, fp);
                
                // Update program state
                rsc_tbl->rsc_descs[resource].allocated[pid]--;

                // Send message back to user program to let it know that we updated the resource table
                send_msg(rsc_msg_box_id, &rsc_msg_box, pid+MAX_PROC_CNT);

                // Check if we can unblock any processes
                unblock_process_if_possible(resource, rsc_msg_box);

            }
            else {
                // Process terminated
                sprintf(buffer, "OSS: Acknowledging that P%d terminated at time %ld:%'ld\n",
                    pid, sysclock->seconds, sysclock->nanoseconds);
                print_and_write(buffer, fp);

                // Update program state
                childpids[pid] = 0;
                proc_cnt--;
                release_resources(rsc_tbl, pid); // Updates resource table
                
                for (i = 0; i < NUM_RSC_CLS; i++) {
                    int j;
                    // Check MAX_CLAIMS times for each resource in case 
                    // the process had it's MAX resources allocated to it
                    for (j = 0; j < MAX_CLAIMS; j++) {
                        // Check if we can unblock any processes
                        unblock_process_if_possible(i, rsc_msg_box);   
                    }
                }

            }
            sprintf(buffer, "\n");
            print_and_write(buffer, fp);

            // Increment clock slightly whenever a resource is granted or released
            increment_clock(sysclock, get_work());
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
    print_rsc_summary(rsc_tbl, fp);

    sprintf(buffer, "\n");
    print_and_write(buffer, fp);
    
    print_blocked_queue();

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
    free(blocked);
    fclose(fp);
    exit(0);
}

struct clock get_time_to_fork_new_proc(struct clock sysclock) {
    unsigned int ns_before_next_proc = rand() % MAX_NS_BEFORE_NEW_PROC; 
    increment_clock(&sysclock, ns_before_next_proc);
    return sysclock;
}

unsigned int get_nanoseconds() {
    return (rand() % 1000000) + 250000; // 250,000 - 1,000,000 inclusive
}

unsigned int get_work() {
    return (rand() % 100000) + 10000; // 10,000 - 100,000 inclusive
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

struct message parse_msg(char* mtext) {
    // Parse a message sent from a user process
    struct message msg;
    char ** msg_info = split_string(mtext, ",");
    
    msg.pid = atoi(msg_info[0]);
    strcpy(msg.txt, msg_info[1]);
    msg.resource = atoi(msg_info[2]);

    free(msg_info);

    return msg;
}

void unblock_process_if_possible(int resource, struct msgbuf rsc_msg_box) {
    if (empty(&blocked[resource])) {
        // There are no processes blocked on this resource
        return;
    }

    bool rsc_is_available = resource_is_available(rsc_tbl, resource);
    if (!rsc_is_available) {
        // Resource is unavaible
        return;
    }

    char reason[50];
    char buffer [100];

    int pid = peek(&blocked[resource]);

    // Resource is available so run bankers algorithm to check if we can
    // safely grant this request
    bool rsc_granted = bankers_algorithm(rsc_tbl, pid, resource);
    increment_clock(sysclock, get_work());
    sprintf(reason, "granting this resource would lead to an unsafe state");
    
    if (rsc_granted) {
        // Resource granted
        sprintf(buffer, "OSS: Unblocking P%d and granting it R%d at time %ld:%'ld\n",
            pid, resource+1, sysclock->seconds, sysclock->nanoseconds);
        print_and_write(buffer, fp);

        // Update program state
        rsc_tbl->rsc_descs[resource].allocated[pid]++;
        num_resources_granted++;
        dequeue(&blocked[resource]);
        
        if (num_resources_granted % 20 == 0) {
            // Print table of allocated resources
            print_allocated_rsc_tbl(rsc_tbl, fp);
        }

        // Send message back to user program to let it know that it's request was granted
        send_msg(rsc_msg_box_id, &rsc_msg_box, pid+MAX_PROC_CNT);
    }
}

void print_blocked_queue() {
    int i;
    bool queue_is_empty = 1;
    char buffer[1000];
    char* queue;
    
    sprintf(buffer, "Blocked Processes\n");
    for (i = 0; i < NUM_RSC_CLS; i++) {
        if (empty(&blocked[i])) {
            // Queue empty
            continue;
        }
        
        // Resource label
        sprintf(buffer + strlen(buffer), "  R%2d:", i+1);
        
        // What processes are blocked on resource i
        queue = get_queue_string(&blocked[i]);
        sprintf(buffer + strlen(buffer), "%s", queue);
        free(queue);
        
        // Queue is not empty
        queue_is_empty = 0;
    }
    
    if (queue_is_empty) {
        sprintf(buffer + strlen(buffer), "  <no blocked processes>\n");
    }
    
    sprintf(buffer + strlen(buffer), "\n");

    print_and_write(buffer, fp);
}