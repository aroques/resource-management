#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdbool.h>
#include <stdio.h>

#define NUM_RSC_CLS 20
#define MAX_PROC_CNT 18
#define MAX_CLAIMS 2 // Upper bound of max number of a resource class a process can claim 

struct resource_descriptor {
    unsigned int total;
    unsigned int allocated[MAX_PROC_CNT+1];
};

struct resource_table {
    struct resource_descriptor rsc_descs[NUM_RSC_CLS];
    unsigned int max_claims[MAX_PROC_CNT+1];
};

void allocate_rsc_tbl(struct resource_table* rsc_tbl);
struct resource_descriptor get_rsc_desc();
void init_allocated(unsigned int* allocated);
unsigned int get_num_resources();
unsigned int get_max_resource_claims();
void release_resources(struct resource_table* rsc_tbl, int pid);
void print_available_rsc_tbl(struct resource_table* rsc_tbl, FILE* fp);
void print_allocated_rsc_tbl(struct resource_table* rsc_tbl, FILE* fp);
unsigned int* get_current_alloced_rscs(int pid, struct resource_table* rsc_tbl);
unsigned int get_number_of_allocated_rsc_classes(int pid, struct resource_table* rsc_tbl);
bool has_resource(int pid, struct resource_table* rsc_tbl);
bool resource_is_available(struct resource_table* rsc_tbl, int requested_resource);
unsigned int* get_allocated_resources(struct resource_table* rsc_tbl);
unsigned int* get_available_resources(struct resource_table* rsc_tbl);
unsigned int* get_total_resources(struct resource_table* rsc_tbl);
void print_resources(unsigned int* resources, char* title, FILE* fp);
void print_rsc_summary(struct resource_table* rsc_tbl, FILE* fp);

#endif
