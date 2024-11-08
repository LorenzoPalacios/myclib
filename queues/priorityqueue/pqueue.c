#include "pqueue.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

/*
 * This defines what percentage of the queue's memory will be allocated as a
 * buffer to efficiently handle enqueued values with high priority.
 */
#define P_QUEUE_RESERVE_PERCENT (.2)

priority_queue *_new_p_queue(const void *const data, const size_t num_elems,
                             const size_t value_size) {
  priority_queue *queue;
  {
    const size_t VALUES_ALLOC = num_elems * value_size;
    const size_t MEMBERS_ALLOC = num_elems * sizeof(p_queue_member);
    const size_t RESERVE_ALLOC = P_QUEUE_RESERVE_PERCENT * VALUES_ALLOC;
    const size_t PADDING_BYTES =
        alignof(p_queue_member) - ((VALUES_ALLOC) % alignof(p_queue_member));
    const size_t TOTAL_ALLOC = MEMBERS_ALLOC + VALUES_ALLOC + RESERVE_ALLOC +
                               PADDING_BYTES + sizeof(priority_queue);
    queue = malloc(TOTAL_ALLOC);
    if (queue == NULL) return NULL;

    /* The values will be stored directly after the queue header. */
    queue->values = queue + 1;
    /*
     * The members will be stored directly after the values, which may require
     * padding to avoid memory alignment issues.
     */
    queue->members =
        (void *)((byte *)queue->values + VALUES_ALLOC + PADDING_BYTES);
    queue->value_size = value_size;
    queue->allocation = TOTAL_ALLOC;
    queue->used_allocation = queue->allocation;
    queue->padding_bytes = PADDING_BYTES;
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

int main(void) {
  char data[] = {1, 2, 3};
  priority_queue *a = new_p_queue(data);
  printf("%zu", p_queue_get_length(a));
  return 0;
}
