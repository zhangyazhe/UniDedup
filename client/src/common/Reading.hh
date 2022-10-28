/**
 * Read the file into fixed size blocks from disk to system.
 */
#ifndef _READING_HH_
#define _READING_HH_

#include "Chunking.hh"
#include "../util/sync_queue.hh"

static void read_file(const char* buf);
static void *read_thread(void *argv);
void start_read_phase(const char* buf);
void stop_read_phase();

#endif