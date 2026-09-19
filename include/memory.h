#ifndef AMNESIC_MEMORY_H
#define AMNESIC_MEMORY_H

#include <stddef.h>
#include <stdbool.h>

bool lock_process_memory(void);
void secure_wipe(void *v, size_t n);

#endif
