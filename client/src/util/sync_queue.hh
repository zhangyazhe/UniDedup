#ifndef _SYNC_QUEUE_HH_
#define _SYNC_QUEUE_HH_

#include <stdlib.h>
#include <pthread.h>
#include "queue.hh"

struct SyncQueue{
	int term; // terminated
	int max_size;/* the max size of queue */
	struct Queue *queue;
	pthread_mutex_t mutex;
	pthread_cond_t max_work;
	pthread_cond_t min_work;
};

struct SyncQueue* sync_queue_new(int);
void sync_queue_free(SyncQueue*, void (*)(void*));
void sync_queue_push(SyncQueue*, void*);
void* sync_queue_pop(SyncQueue*);
void sync_queue_term(SyncQueue*);
int sync_queue_size(SyncQueue* s_queue);
void* sync_queue_find(SyncQueue* s_queue, int (*hit)(void*, void*), void* data,
		void* (*dup)(void*));
void* sync_queue_get_top(SyncQueue* s_queue);

#endif