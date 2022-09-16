#ifndef _ASSEMBLING_HH_
#define _ASSEMBLING_HH_

#include "Receiving.hh"
#include "Chunking.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include "../util/sync_queue.hh"

/* Define the length of the assembly area, the default value is 30 4MB container size */
#define ASSEMBLE_BUFFER_LEN 30 * 4194304ll

void start_assemble_phase(const char *path);
void stop_assemble_phase(void);

#endif