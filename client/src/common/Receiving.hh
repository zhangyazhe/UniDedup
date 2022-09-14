#ifndef _RECEIVING_HH_
#define _RECEIVING_HH_

#include "Chunking.hh"
#include "../util/sync_queue.hh"
#include "FileRecipe.hh"
#include "../util/RedisUtil.hh"

extern SyncQueue *receive_queue;

static void receive_data(const fileRecipe* fr);
static void *receive_thread(void *argv);
void start_receive_phase(const char* buf);
void stop_receive_phase();

#endif