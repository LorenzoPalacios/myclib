#include "pqueue.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

/*
 * This defines a percentage of required queue members (from the total) to be
 * added as extraneous members to facilitate the efficient enqueuing of members
 * with high priority.
 */
#define PERCENT_RESERVED_MEMBERS (0)

static priority_queue *instantiate_p_queue(const size_t num_members,
                                           const size_t value_size,
                                           const size_t num_reserve_members) {
  const size_t TOTAL_MEMBERS = num_members + num_reserve_members;

  const size_t VALUES_ALLOC = TOTAL_MEMBERS * value_size;
  const size_t MEMBERS_ALLOC = TOTAL_MEMBERS * sizeof(p_queue_member);
  /* This is only used to calculate the used allocation. */
  const size_t RESERVED_ALLOC = num_reserve_members * (sizeof(p_queue_member) + value_size);

  const size_t ALIGNMENT_DIFF = VALUES_ALLOC % alignof(p_queue_member);
  const size_t PADDING_BYTES =
      (ALIGNMENT_DIFF != 0) ? (alignof(p_queue_member) - ALIGNMENT_DIFF) : (0);

  const size_t TOTAL_ALLOC =
      MEMBERS_ALLOC + VALUES_ALLOC + PADDING_BYTES + sizeof(priority_queue);
  priority_queue *const queue = malloc(TOTAL_ALLOC);
  if (queue == NULL) return NULL;

  /* The values will be stored directly after the queue header. */
  queue->values = queue + 1;
  /*
   * The members will be stored directly after the values, which may require
   * padding to avoid misalignment.
   */
  queue->members =
      (void *)((byte *)queue->values + VALUES_ALLOC + PADDING_BYTES);
  queue->value_size = value_size;
  queue->allocation = TOTAL_ALLOC;
  queue->used_allocation =
      TOTAL_ALLOC - RESERVED_ALLOC - PADDING_BYTES + sizeof(priority_queue);
  queue->padding_bytes = PADDING_BYTES;

  return queue;
}

priority_queue *_new_p_queue(const void *const data, const size_t num_values,
                             const size_t value_size) {
  const size_t RESERVED_MEMBERS = PERCENT_RESERVED_MEMBERS * num_values;
  priority_queue *queue =
      instantiate_p_queue(num_values, value_size, RESERVED_MEMBERS);

  void *const values = queue->values;
  p_queue_member *const members = queue->members;

  return queue;
}

size_t p_queue_get_length(const priority_queue *const queue) {
  const size_t MEMBERS_ALLOC = queue->used_allocation - sizeof(priority_queue);
  const size_t TOTAL_MEMBER_SIZE = queue->value_size + sizeof(p_queue_member);
  return MEMBERS_ALLOC / TOTAL_MEMBER_SIZE;
}

size_t p_queue_get_capacity(const priority_queue *const queue) {
  const size_t NOT_MEMBERS_ALLOC = queue->allocation - queue->used_allocation;
  const size_t TOTAL_MEMBER_SIZE = queue->value_size + sizeof(p_queue_member);
  return NOT_MEMBERS_ALLOC / TOTAL_MEMBER_SIZE;
}

int main(void) {
  int data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  priority_queue *a = new_p_queue(data);
  printf("%zu %zu\n", a->allocation, a->used_allocation);
  printf("%zu\n", p_queue_get_length(a));
  printf("%zu", p_queue_get_capacity(a));
  return 0;
}
