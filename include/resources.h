#ifndef RESOURCES_H
#define RESOURCES_H

#define NUM_RSC_CLS 20
#define MAX_PROC_CNT 18

struct resource_descriptor {
    unsigned int total;
    unsigned int allocated[MAX_PROC_CNT+1];
};

struct resource_table {
    struct resource_descriptor rsc_descs[NUM_RSC_CLS];
    unsigned int max_claims[MAX_PROC_CNT+1];
};

void print_allocated_rsc_tbl(struct resource_table* rsc_tbl);
void allocate_rsc_tbl(struct resource_table* rsc_tbl);
struct resource_descriptor get_rsc_desc();
void init_allocated(unsigned int* allocated);
unsigned int get_num_resources();
unsigned int get_max_resource_claims();

#endif
