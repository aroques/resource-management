#include "bankers.h"
#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>

#define NUM_RSC_CLS 20
#define MAX_PROC_CNT 18

bool bankers_algorithm(struct resource_table* rsc_tbl, unsigned int requested_resource) {
    unsigned int i, j;

    // Suppose P1 requests resource 4.
    // Then we add it to P1's allocation and subtract it from avaiable
    
    unsigned int* total_resources = get_total_resources(rsc_tbl);
    unsigned int* allocated_resources = get_allocated_resources(rsc_tbl);
    unsigned int* available_resources = get_available_resources(total_resources, allocated_resources); 

    // Load max
    unsigned int max[NUM_RSC_CLS];
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        max[i] += rsc_tbl->max_claims[i];
    }

    unsigned int** needs = get_needs_matrix(rsc_tbl);

    // Copy avaiable into work
    unsigned work[NUM_RSC_CLS];
    for (i = 0; i < NUM_RSC_CLS; i++) {
        work[i] = available_resources[i];
    }

    // Initialize can finish to true
    bool can_finish[MAX_PROC_CNT+1];
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        can_finish[i] = 1;
    }

    for (i = 1; i <= MAX_PROC_CNT; i++) {
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
        }
    }

    free(total_resources);
    free(allocated_resources);
    free(available_resources);
    destroy_array(needs);

    // https://www.youtube.com/watch?v=w0LwGqffUkg
    return 0;
}

unsigned int* get_available_resources(unsigned int* total_resources, unsigned int* allocated_resources) {
    // Subtract the two to get the total available resources
    unsigned int i;
    unsigned int* available_resources = malloc(sizeof(unsigned int) * NUM_RSC_CLS);
    for (i = 0; i < NUM_RSC_CLS; i++) {
        available_resources[i] = total_resources[i] - allocated_resources[i];
    }
    return available_resources;
}

unsigned int* get_total_resources(struct resource_table* rsc_tbl) {
    unsigned int i;
    unsigned int* total_resources = malloc(sizeof(unsigned int) * NUM_RSC_CLS);
    for (i = 0; i < NUM_RSC_CLS; i++) {
        total_resources[i] = rsc_tbl->rsc_descs[i].total;
    }
    return total_resources;
}

unsigned int* get_allocated_resources(struct resource_table* rsc_tbl) {
    unsigned int i, j;
    unsigned int* allocated_resources = malloc(sizeof(unsigned int) * NUM_RSC_CLS);
    for (i = 0; i < NUM_RSC_CLS; i++) {
        for (j = 1; j <= MAX_PROC_CNT; j++) {
            allocated_resources[i] += rsc_tbl->rsc_descs[i].allocated[j];
        }
    }
    return allocated_resources;
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