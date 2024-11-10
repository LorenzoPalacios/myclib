#ifndef PQUEUE_H
#define PQUEUE_H

#include <stddef.h>

/*
 * Actual priority levels are not required to use these values, but should
 * be taken as a suggestion for what priority values should look like.
 */
typedef enum {
  HIGH = 0,
  MEDIUM = 1,
  LOW = 2,
} BASE_PRIORITY_LEVELS;

typedef struct {
  size_t value_access_offset;
  size_t priority;
} p_queue_member;

typedef struct {
  void *begin_values;
  p_queue_member *begin_members;
  p_queue_member *front;
  p_queue_member *back;
  size_t value_size;
  size_t allocation;
  size_t used_allocation;
  size_t padding_bytes;
} priority_queue;

#define new_p_queue(data) \
  _new_p_queue(data, sizeof(data) / sizeof *(data), sizeof *(data))

#define new_p_queue_with_buf(data, buf_size) \
  _new_p_queue_with_buf(data, sizeof(data) / sizeof *(data), sizeof *(data))

priority_queue *_new_p_queue(const void *data, size_t num_elems,
                             size_t elem_size);

priority_queue *_new_p_queue_with_buf(const void *data, size_t num_elems,
                                      size_t elem_size, size_t buf_size);

priority_queue *p_queue_enqueue(priority_queue *queue, const void *elem);

void *p_queue_dequeue(priority_queue *queue);

void *p_queue_front(priority_queue *queue);

void *p_queue_back(priority_queue *queue);

/* \return the number of elements currently enqueued within `queue`. */
size_t p_queue_get_length(const priority_queue *queue);

/*
 * \return the maximum number of elements that can be held by `queue` before
 * expansion is necessary.
 */
size_t p_queue_get_capacity(const priority_queue *queue);

priority_queue *p_queue_resize(priority_queue *queue, size_t new_size);

priority_queue *p_queue_expand(priority_queue *queue);

priority_queue *p_queue_shrink(priority_queue *queue);

#endif
