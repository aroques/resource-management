#include <stdio.h>
#include <stdlib.h>

#include "resources.h"

void print_available_rsc_tbl(struct resource_table* rsc_tbl) {
    int i, j, available;
    printf("\n");
    printf("%61s", "Current Available System Resources\n");
    printf("     ");
    // print column titles
    for (i = 0; i < NUM_RSC_CLS; i++) {
        printf("R%-3d", i+1);
    }
    printf("\n");
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        printf("P%-4d", i);
        // print all resources allocated for process i
        for (j = 0; j < NUM_RSC_CLS; j++) {
            available = rsc_tbl->rsc_descs[j].total - rsc_tbl->rsc_descs[j].allocated[i];
            printf("%-4d", available);
        }
        printf("\n");
    }
    printf("\n");
}

void print_allocated_rsc_tbl(struct resource_table* rsc_tbl) {
    int i, j;
    printf("\n");
    printf("%61s", "Current (Allocated) System Resources\n");
    printf("     ");
    // print column titles
    for (i = 0; i < NUM_RSC_CLS; i++) {
        printf("R%-3d", i+1);
    }
    printf("\n");
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        printf("P%-4d", i);
        // print all resources allocated for process i
        for (j = 0; j < NUM_RSC_CLS; j++) {
            printf("%-4d", rsc_tbl->rsc_descs[j].allocated[i]);
        }
        printf("\n");
    }
    printf("\n");
}

void allocate_rsc_tbl(struct resource_table* rsc_tbl) {
    int i;
    for (i = 0; i < NUM_RSC_CLS; i++) {
        rsc_tbl->rsc_descs[i] = get_rsc_desc();
    }
}

struct resource_descriptor get_rsc_desc() {
    struct resource_descriptor rsc_desc = {
        .total = get_num_resources(),
    };
    init_allocated(rsc_desc.allocated);
    return rsc_desc;
}

unsigned int get_num_resources() {
    return (rand() % 10) + 1; // 1 - 10 inclusive
}

void init_allocated(unsigned int* allocated) {
    int i;
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        allocated[i] = 0;
    }
}

unsigned int get_max_resource_claims() {
    const unsigned int MAX_CLAIMS = 4; // Upper bound of max number of a resource class a process can claim 
    return (rand() % MAX_CLAIMS) + 1;
}

void release_resources(struct resource_table* rsc_tbl, int pid) {
    unsigned int i;
    for (i = 0; i < NUM_RSC_CLS; i++) {
        rsc_tbl->rsc_descs[i].allocated[pid]--;    
    }
}

unsigned int* get_current_alloced_rscs(int pid, struct resource_table* rsc_tbl) {
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
