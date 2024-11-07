#include "pqueue.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

/* 
 * This defines what percentage of the queue's memory will be allocated as a
 * buffer to efficiently handle enqueued values with high priority.
 */
#define P_QUEUE_BUF_PERCENT (.2)

priority_queue *_new_p_queue(const void *const data, const size_t num_elems,
                             const size_t value_size) {
  priority_queue *queue;
  {
    const size_t VALUES_ALLOC = num_elems * value_size;
    const size_t MEMBERS_ALLOC = num_elems * sizeof(p_queue_member);
    const size_t TOTAL_ALLOC =
        MEMBERS_ALLOC + VALUES_ALLOC + sizeof(priority_queue);

    queue = malloc(TOTAL_ALLOC);
    if (queue == NULL) return NULL;

    /* The values will be stored directly after the queue header. */
    queue->values = queue + 1;
    /* The members will be stored directly after the values. */
    queue->members = (void *)((byte *)queue->values + VALUES_ALLOC);
    queue->value_size = value_size;
    queue->allocation = TOTAL_ALLOC;
    queue->used_allocation = queue->allocation;
  }

  void *const values = queue->values;
  p_queue_member *const members = queue->members;

  return queue;
}

size_t p_queue_get_length(const priority_queue *const queue) {
  return (queue->used_allocation - sizeof(priority_queue)) / queue->value_size;
}

size_t p_queue_get_capacity(const priority_queue *const queue) {
  const size_t QUEUE_VALUE_ALLOC =
      queue->allocation - queue->used_allocation - sizeof(priority_queue);
  return QUEUE_VALUE_ALLOC / queue->value_size;
}
