#ifndef _RECEIVING_HH_
#define _RECEIVING_HH_

#include "Chunking.hh"
#include "../util/sync_queue.hh"
#include "FileRecipe.hh"
#include "../util/RedisUtil.hh"

extern SyncQueue *receive_queue;

static void receive_data(struct fileRecipe* fr);
static void *receive_thread(struct fileRecipe* fr);
void start_receive_phase(struct fileRecipe * buf);
void stop_receive_phase();

#endif