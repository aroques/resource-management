#ifndef BANKERS_H
#define BANKERS_H

#include "shared_memory.h"
#include <stdbool.h>

bool bankers_algorithm(struct resource_table* rsc_tbl, unsigned int requested_resource);
unsigned int* get_total_resources(struct resource_table* rsc_tbl);
unsigned int* get_allocated_resources(struct resource_table* rsc_tbl);
unsigned int* get_available_resources(unsigned int* total_resources, unsigned int* allocated_resources);
unsigned int** get_needs_matrix(struct resource_table* rsc_tbl);

#endif
