#ifndef _HASHING_HH_
#define _HASHING_HH_

#include "Group.hh"
#include "Fingerprint.hh"
#include "../util/sync_queue.hh"

static void* sha1_thread(void *arg);
void start_hash_phase();
void stop_hash_phase();

#endif