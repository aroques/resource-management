#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resources.h"
#include "helpers.h"

#define BUF_SIZE 1800

void print_rsc_summary(struct resource_table* rsc_tbl, FILE* fp) {
    unsigned int* total_resources = get_total_resources(rsc_tbl);
    unsigned int* allocated_resources = get_allocated_resources(rsc_tbl);
    unsigned int* available = get_available_resources(rsc_tbl);

    char buffer[100];

    sprintf(buffer, "%48s", "Resource Summary\n");
    print_and_write(buffer, fp);

    print_resources(total_resources, "Total Resources\n", fp);
    print_resources(allocated_resources, "Allocated Resources\n", fp);
    print_resources(available, "Available Resources\n", fp);

    free(total_resources);
    free(allocated_resources);
    free(available);
}

void print_resources(unsigned int* resources, char* title, FILE* fp) {
    int i;
    char buffer[BUF_SIZE];
    
    // Print title
    sprintf(buffer, "\n");
    sprintf(buffer + strlen(buffer), "%s", title);
    
    // Print column titles
    sprintf(buffer + strlen(buffer), "  ");
    for (i = 0; i < NUM_RSC_CLS; i++) {
        sprintf(buffer + strlen(buffer),"R%-3d", i+1);
    }
    sprintf(buffer + strlen(buffer),"\n");
    
    // Print data
    sprintf(buffer + strlen(buffer), "  ");
    for (i = 0; i < NUM_RSC_CLS; i++) {
        sprintf(buffer + strlen(buffer),"%-4d", resources[i]);
    }
    sprintf(buffer + strlen(buffer),"\n");
    print_and_write(buffer, fp);
}

void print_allocated_rsc_tbl(struct resource_table* rsc_tbl, FILE* fp) {
    int i, j;
    char buffer[BUF_SIZE];
    sprintf(buffer,"\n");
    sprintf(buffer + strlen(buffer),"%61s", "Current (Allocated) System Resources\n");
    sprintf(buffer + strlen(buffer),"     ");
    // print column titles
    for (i = 0; i < NUM_RSC_CLS; i++) {
        sprintf(buffer + strlen(buffer),"R%-3d", i+1);
    }
    sprintf(buffer + strlen(buffer),"\n");
    for (i = 1; i <= MAX_PROC_CNT; i++) {
        sprintf(buffer + strlen(buffer),"P%-4d", i);
        // print all resources allocated for process i
        for (j = 0; j < NUM_RSC_CLS; j++) {
            sprintf(buffer + strlen(buffer),"%-4d", rsc_tbl->rsc_descs[j].allocated[i]);
        }
        sprintf(buffer + strlen(buffer),"\n");
    }
    sprintf(buffer + strlen(buffer),"\n");
    print_and_write(buffer, fp);
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

bool resource_is_available(struct resource_table* rsc_tbl, int requested_resource) {
    unsigned int* allocated_resources = get_allocated_resources(rsc_tbl);
    unsigned int currently_allocated = allocated_resources[requested_resource];
    unsigned int total = rsc_tbl->rsc_descs[requested_resource].total;
    free(allocated_resources);
    if (currently_allocated == total) {
        // All resources in this resource class have already been allocated so
        // the resource is not available
        return 0;
    }
    return 1;
}

unsigned int* get_available_resources(struct resource_table* rsc_tbl) {
    // Subtract the two to get the total available resources
    unsigned int i;
    unsigned int* allocated_resources = get_allocated_resources(rsc_tbl);
    unsigned int* total_resources = get_total_resources(rsc_tbl);
    unsigned int* available_resources = malloc(sizeof(unsigned int) * NUM_RSC_CLS);
    for (i = 0; i < NUM_RSC_CLS; i++) {
        available_resources[i] = total_resources[i] - allocated_resources[i];
    }
    
    free(allocated_resources);
    free(total_resources);
    
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
        allocated_resources[i] = 0;
    }

    for (i = 0; i < NUM_RSC_CLS; i++) {
        for (j = 1; j <= MAX_PROC_CNT; j++) {
            allocated_resources[i] += rsc_tbl->rsc_descs[i].allocated[j];
        }
    }
    return allocated_resources;
}