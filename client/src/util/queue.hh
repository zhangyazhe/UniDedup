/*
 * queue.h
 *
 *  Created on: May 21, 2012
 *      Author: fumin
 */

#ifndef _QUEUE_HH_
#define _QUEUE_HH_

struct queue_ele_t {
	struct queue_ele_t *next;
	void *data;
};

/*
 * Structure describing a queue
 */
struct Queue {
	struct queue_ele_t *first, *last; /* work queue */
	int elem_num;
	//int max_elem_num; //-1 means infi.
};

Queue* queue_new();
void queue_free(Queue *queue, void (*)(void*));
void queue_push(Queue *queue, void *element);
void* queue_pop(Queue *queue);
int queue_size(Queue *queue);
void queue_foreach(Queue *queue, void (*func)(void *data, void *user_data),
		void *user_data);
void* queue_get_n(Queue *queue, int n);
void * queue_top(Queue *queue);
void* queue_find(Queue* queue, int (*hit)(void*, void*), void* data);

#endif /* QUEUE_H_ */
