#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define NUM_RSC_CLS 20
#define MAX_PROC_CNT 18

#include <stdbool.h>
#include "clock.h"
    
struct resource_descriptor {
    unsigned int total;
    unsigned int allocated[MAX_PROC_CNT+1];
};

struct resource_table {
    struct resource_descriptor rsc_descs[NUM_RSC_CLS];
};

int get_shared_memory();
void* attach_to_shared_memory(int shmemid, unsigned int readonly);
void cleanup_shared_memory(int shmemid, void* p);
void detach_from_shared_memory(void* p);
void deallocate_shared_memory(int shmemid);

#endif
