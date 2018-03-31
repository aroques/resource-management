#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>
#include "clock.h"

int get_shared_memory();
void* attach_to_shared_memory(int shmemid, unsigned int readonly);
void cleanup_shared_memory(int shmemid, void* p);
void detach_from_shared_memory(void* p);
void deallocate_shared_memory(int shmemid);

#endif
