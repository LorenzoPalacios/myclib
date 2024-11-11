#ifndef PQUEUE_H
#define PQUEUE_H

#include <stddef.h>

/*
 * Actual priority levels are not required to use these values, but should
 * be taken as a suggestion for how priorities are interpreted by the library.
 */
typedef enum {
  HIGH = 0,
  MEDIUM = 1,
  LOW = 2,
} BASE_PRIORITY_LEVELS;

typedef struct {
  size_t values_offset;
  size_t priority;
} p_queue_member;

typedef struct {
  size_t front_index;
  size_t back_index;
  size_t value_size;
  size_t data_allocation;
} priority_queue;

/* - Queue Creation and Deletion - */

#define new_p_queue(data) \
  _new_p_queue(data, sizeof(data) / sizeof *(data), sizeof *(data))

#define delete_p_queue(queue) _delete_p_queue(&(queue))

#define delete_p_queue_s(queue) _delete_p_queue_s(&(queue))

priority_queue *_new_p_queue(const void *data, size_t num_elems,
                             size_t elem_size);

priority_queue *instantiate_p_queue(size_t num_members, size_t value_size,
                                    size_t num_reserve_members);

void _delete_p_queue(priority_queue **queue);

void _delete_p_queue_s(priority_queue **queue);

/* - Queue Operations - */

void *p_queue_back(priority_queue *queue);

void *p_queue_dequeue(priority_queue *queue);

priority_queue *p_queue_enqueue(priority_queue *queue, const void *elem);

void *p_queue_front(priority_queue *queue);

/* \return the number of elements currently enqueued within `queue`. */
size_t p_queue_get_length(const priority_queue *queue);

/*
 * \return the maximum number of elements that can be held by `queue` before
 * expansion is necessary.
 */
size_t p_queue_get_capacity(const priority_queue *queue);

/* - Queue Memory Manipulation - */

priority_queue *p_queue_expand(priority_queue *queue);

priority_queue *p_queue_resize(priority_queue *queue, size_t new_size);

priority_queue *p_queue_shrink(priority_queue *queue);

#endif
