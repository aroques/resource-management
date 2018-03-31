#include <stdio.h>
#include <stdlib.h>

#include "resources.h"

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