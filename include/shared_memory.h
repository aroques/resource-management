#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define MAX_PROC_CNT 18

#include <stdbool.h>
#include "clock.h"
    
struct resource_descriptor {
    unsigned int total;
    unsigned int allocated;
};

struct resource_table {
    struct resource_descriptor rsc_descs[MAX_PROC_CNT];
};

int get_shared_memory();
void* attach_to_shared_memory(int shmemid, unsigned int readonly);
void cleanup_shared_memory(int shmemid, void* p);
void detach_from_shared_memory(void* p);
void deallocate_shared_memory(int shmemid);

#endif
