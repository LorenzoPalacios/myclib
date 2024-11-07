#ifndef PQUEUE_H
#define PQUEUE_H

#include <stddef.h>

typedef struct {
  void *data;
  size_t data_offset;
  size_t allocation;
  size_t used_allocation;
} priority_queue;

#define new_p_queue(data) \
  _new_p_queue(data, sizeof(data) / sizeof *(data), sizeof *(data))

#define new_p_queue_with_buf(data, buf_size) \
  _new_p_queue_with_buf(data, sizeof(data) / sizeof *(data), sizeof *(data))

priority_queue *_new_p_queue(const void *data, size_t num_elems,
                             size_t elem_size);

priority_queue *_new_p_queue_with_buf(const void *data, size_t num_elems,
                                      size_t elem_size, size_t buf_size);

priority_queue *p_queue_enqueue(priority_queue *queue, void *elem);

void *p_queue_dequeue(priority_queue *queue);

void *p_queue_front(priority_queue *queue);

void *p_queue_back(priority_queue *queue);

size_t p_queue_get_length(priority_queue *queue);

size_t p_queue_get_capacity(priority_queue *queue);

priority_queue *p_queue_resize(priority_queue *queue, size_t new_size);

priority_queue *p_queue_expand(priority_queue *queue);

priority_queue *p_queue_shrink(priority_queue *queue);

#endif
