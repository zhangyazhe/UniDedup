#include "sync_queue.hh"
#include <stdio.h>
#include <iostream>

SyncQueue* sync_queue_new(int size) {
	// std::cout << "[debug]: 32211" << std::endl;
	SyncQueue *s_queue = (SyncQueue*) malloc(sizeof(SyncQueue));
	// std::cout << "[debug]: 32212" << std::endl;
	s_queue->queue = queue_new();
	// std::cout << "[debug]: 32213" << std::endl;
	s_queue->max_size = size;
	s_queue->term = 0;

	if (pthread_mutex_init(&s_queue->mutex, 0)
			|| pthread_cond_init(&s_queue->max_work, 0)
			|| pthread_cond_init(&s_queue->min_work, 0)) {
		puts("Failed to init mutex or work in SyncQueue!");
		return NULL;
	}
	// std::cout << "[debug]: 32214" << std::endl;
	return s_queue;
}

void sync_queue_free(SyncQueue* s_queue, void (*free_data)(void*)) {
	if(s_queue == NULL) {
		std::cout << "sync_queue_free null" << std::endl;
		return;
	}
	queue_free(s_queue->queue, free_data);
	pthread_mutex_destroy(&s_queue->mutex);
	pthread_cond_destroy(&s_queue->max_work);
	pthread_cond_destroy(&s_queue->min_work);
	free(s_queue);
}

void sync_queue_push(SyncQueue* s_queue, void* item) {
	if(s_queue == NULL) {
		std::cout << "sync_queue_push s_queue NULL" << std::endl;
		return;
	}
	if (pthread_mutex_lock(&s_queue->mutex) != 0) {
		puts("failed to lock!");
		return;
	}

	if (s_queue->term == 1) {
		pthread_mutex_unlock(&s_queue->mutex);
		return;
	}

	while (s_queue->max_size > 0
			&& queue_size(s_queue->queue) >= s_queue->max_size) {
		pthread_cond_wait(&s_queue->max_work, &s_queue->mutex);
	}

	queue_push(s_queue->queue, item);

	pthread_cond_broadcast(&s_queue->min_work);

	if (pthread_mutex_unlock(&s_queue->mutex)) {
		puts("failed to lock!");
		return;
	}
}

/*
 * Return NULL if the queue is terminated.
 */
void* sync_queue_pop(SyncQueue* s_queue) {
	if(s_queue == NULL) {
		std::cout << "sync_queue_pop s_queue NULL" << std::endl;
		return NULL;
	}
	if (pthread_mutex_lock(&s_queue->mutex) != 0) {
		puts("failed to lock!");
		return NULL;
	}

	while (queue_size(s_queue->queue) == 0) {
		if (s_queue->term == 1) {
			pthread_mutex_unlock(&s_queue->mutex);
			return NULL;
		}
		pthread_cond_wait(&s_queue->min_work, &s_queue->mutex);
	}

	void * item = queue_pop(s_queue->queue);
	pthread_cond_broadcast(&s_queue->max_work);

	pthread_mutex_unlock(&s_queue->mutex);
	return item;
}

int sync_queue_size(SyncQueue* s_queue) {
	if(s_queue == NULL) {
		std::cout << "sync_queue_size s_queue NULL" << std::endl;
		return 0;
	}
	return queue_size(s_queue->queue);
}

void sync_queue_term(SyncQueue* s_queue) {
	if(s_queue == NULL) {
		std::cout << "sync_queue_term s_queue NULL" << std::endl;
		return;
	}
	if (pthread_mutex_lock(&s_queue->mutex) != 0) {
		puts("failed to lock!");
		return;
	}

	s_queue->term = 1;

	pthread_cond_broadcast(&s_queue->min_work);

	pthread_mutex_unlock(&s_queue->mutex);
}

/*
 * Iterate the Queue to find an elem which meets the condition ('hit' returns 1).
 */
void* sync_queue_find(SyncQueue* s_queue, int (*hit)(void*, void*), void* data,
		void* (*dup)(void*)) {
	void* ret = NULL;
	if(s_queue == NULL) {
		std::cout << "sync_queue_find s_queue NULL" << std::endl;
		return NULL;
	}
	if (pthread_mutex_lock(&s_queue->mutex) != 0) {
		puts("failed to lock!");
		return NULL;
	}

	ret = queue_find(s_queue->queue, hit, data);

	if (ret && dup) {
		/* Create a copy */
		ret = dup(ret);
	}

	pthread_mutex_unlock(&s_queue->mutex);

	return ret;
}

void* sync_queue_get_top(SyncQueue* s_queue) {
	if(s_queue == NULL) {
		std::cout << "sync_queue_get_top s_queue NULL" << std::endl;
		return NULL;
	}
	if (pthread_mutex_lock(&s_queue->mutex) != 0) {
		puts("failed to lock!");
		return NULL;
	}

	while (queue_size(s_queue->queue) == 0) {
		if (s_queue->term == 1) {
			pthread_mutex_unlock(&s_queue->mutex);
			return NULL;
		}
		pthread_cond_wait(&s_queue->min_work, &s_queue->mutex);
	}

	void * item = queue_top(s_queue->queue);

	pthread_mutex_unlock(&s_queue->mutex);
	return item;
}
