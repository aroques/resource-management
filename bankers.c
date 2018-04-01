#include "bankers.h"
#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>

#define NUM_RSC_CLS 20
#define MAX_PROC_CNT 18

bool bankers_algorithm(struct resource_table* rsc_tbl, int pid, unsigned int requested_resource) {
    unsigned int i, j;
    
    // We grant the request and then check if there is a safe state
    rsc_tbl->rsc_descs[requested_resource].allocated[pid]++;

    unsigned int* available_resources = get_available_resources(rsc_tbl); 
    unsigned int** needs = get_needs_matrix(rsc_tbl);
    unsigned int* work = get_work_arr(available_resources);
    bool* can_finish = get_can_finish();

    /*
     *  Determine if there is a safe sequence
     */
    unsigned int num_that_could_finish = 0;

    do {
        num_that_could_finish = 0;

        // Check if each process can finish executing
        // If it can then add its allocated resources to the work vector
        for (i = 1; i <= MAX_PROC_CNT; i++) {
            if (can_finish[i]) {
                // We've already determined that this process can finish
                continue;
            }
            // For process i
            for(j = 0; j < NUM_RSC_CLS; j++) {
                // Check if needs is greater than available
                if (needs[i][j] > available_resources[j]) {
                    // If it is then we cannot finish executing
                    can_finish[i] = 0;
                    break;
                }
            }
            if (can_finish[i]) {
                // Can finish so add process i's allocated resources to the work vector
                for (j = 0; j < NUM_RSC_CLS; j++) {
                    work[j] += rsc_tbl->rsc_descs[j].allocated[i];
                }
                num_that_could_finish++;
            }
        }

    } while (num_that_could_finish > 0);

    bool safe_sequence_exists = check_for_safe_sequence(can_finish);
    
    // Restore resource table state
    rsc_tbl->rsc_descs[requested_resource].allocated[pid]--;
    
    free(available_resources);
    free(work);
    free(can_finish);
    destroy_array(needs);

    return safe_sequence_exists;
}

unsigned int* get_work_arr(unsigned int* available_resources) {
    unsigned int i;
    unsigned int* work = malloc(sizeof(unsigned int) * NUM_RSC_CLS);
    for (i = 0; i < NUM_RSC_CLS; i++) {
        work[i] = available_resources[i];
    }
    return work;
}

bool* get_can_finish() {
    unsigned int i;
    bool* can_finish = malloc(sizeof(bool) * MAX_PROC_CNT+1);
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        can_finish[i] = 1;
    }
    return can_finish;
}

unsigned int** get_needs_matrix(struct resource_table* rsc_tbl) {
    unsigned int i, j;
    unsigned int** needs = create_array(MAX_PROC_CNT+1, NUM_RSC_CLS);
    unsigned int max_processes, allocated_processes;
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        for (j = 0; j < NUM_RSC_CLS; j++) {
            max_processes = rsc_tbl->max_claims[i];                   // Max number of resources for process i
            allocated_processes = rsc_tbl->rsc_descs[j].allocated[i]; // Number of allocated resources for process i
            needs[i][j] = max_processes - allocated_processes;
        }
    }
    return needs;
}

bool check_for_safe_sequence(bool* can_finish) {
    unsigned int i;
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        if (!can_finish[i]) {
            return 0;
        }
    }
    return 1;
}