#ifndef BANKERS_H
#define BANKERS_H

#include "resources.h"
#include <stdbool.h>

bool bankers_algorithm(struct resource_table* rsc_tbl, int pid, unsigned int requested_resource);
unsigned int* get_work_arr(unsigned int* available_resources);
bool* get_can_finish();
bool check_for_safe_sequence(bool* can_finish);
unsigned int** get_needs_matrix(struct resource_table* rsc_tbl);

#endif
