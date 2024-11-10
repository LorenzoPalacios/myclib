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
#define PERCENT_RESERVED_MEMBERS (0.125)

static inline size_t calc_member_padding(const size_t size) {
  const size_t ALIGNMENT_DIFF = size % alignof(p_queue_member);
  return (ALIGNMENT_DIFF != 0) ? (alignof(p_queue_member) - ALIGNMENT_DIFF) : 0;
}

static priority_queue *instantiate_p_queue(const size_t num_members,
                                           const size_t value_size,
                                           const size_t num_reserve_members) {
  const size_t TOTAL_MEMBERS = num_members + num_reserve_members;

  const size_t VALUES_ALLOC = TOTAL_MEMBERS * value_size;
  const size_t MEMBERS_ALLOC = TOTAL_MEMBERS * sizeof(p_queue_member);
  /* This is only used to calculate the used allocation. */
  const size_t RESERVED_ALLOC =
      num_reserve_members * (sizeof(p_queue_member) + value_size);

  const size_t PADDING_BYTES = calc_member_padding(VALUES_ALLOC);

  const size_t TOTAL_ALLOC =
      MEMBERS_ALLOC + VALUES_ALLOC + PADDING_BYTES + sizeof(priority_queue);
  priority_queue *const queue = malloc(TOTAL_ALLOC);
  if (queue == NULL) return NULL;

  /* The values will be stored directly after the queue header. */
  queue->begin_values = queue + 1;
  /*
   * The members will be stored directly after the values, which may require
   * padding to avoid misalignment.
   */
  queue->begin_members =
      (void *)((byte *)queue->begin_values + VALUES_ALLOC + PADDING_BYTES);
  queue->front_offset = 0;
  queue->back_offset = 0;
  queue->value_size = value_size;
  queue->allocation = TOTAL_ALLOC;
  queue->used_allocation = TOTAL_ALLOC - RESERVED_ALLOC - PADDING_BYTES;
  queue->padding_bytes = PADDING_BYTES;

  return queue;
}

static inline void *get_member_value(priority_queue *const queue,
                                     const p_queue_member *const member) {
  return (byte *)queue->begin_values + member->value_access_offset;
}

/*
 * The pointer to `queue` is not `const` because my formatter (clang-format)
 * makes this function look weird whenever the `const` qualifier is applied to
 * it.
 */
static inline size_t get_overall_member_size(const priority_queue *queue) {
  return queue->value_size + sizeof(p_queue_member);
}

priority_queue *_new_p_queue(const void *const data, const size_t num_values,
                             const size_t value_size) {
  const size_t RESERVED_MEMBERS = PERCENT_RESERVED_MEMBERS * num_values;
  priority_queue *const queue =
      instantiate_p_queue(num_values, value_size, RESERVED_MEMBERS);

  /*
   * Reserved members should not be written to during initialization, so we skip
   * over them.
   */
  p_queue_member *const non_reserve_members = queue->begin_members + RESERVED_MEMBERS;

  for (size_t i = 0; i < num_values; i++) {
    p_queue_member *const cur_member = non_reserve_members + i;
    const size_t offset = i * value_size;
    cur_member->priority = MEDIUM;
    cur_member->value_access_offset = offset;

    void *const cur_member_val = get_member_value(queue, cur_member);
    memcpy(cur_member_val, (byte *)data + i * value_size, value_size);
  }
  queue->front_offset = non_reserve_members - queue->begin_members;
  queue->back_offset = queue->front_offset + num_values;

  return queue;
}

size_t p_queue_get_length(const priority_queue *const queue) {
  const size_t USED_ALLOC = queue->used_allocation - sizeof(priority_queue);
  const size_t TOTAL_MEMBER_SIZE = queue->value_size + sizeof(p_queue_member);
  return USED_ALLOC / TOTAL_MEMBER_SIZE;
}

size_t p_queue_get_capacity(const priority_queue *const queue) {
  const size_t UNUSED_ALLOC = queue->allocation - queue->used_allocation;
  const size_t TOTAL_MEMBER_SIZE = queue->value_size + sizeof(p_queue_member);
  return UNUSED_ALLOC / TOTAL_MEMBER_SIZE;
}

void *p_queue_front(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  return get_member_value(queue, queue->begin_members + queue->front_offset);
}

void *p_queue_back(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  return get_member_value(queue, queue->begin_members + queue->back_offset);
}

void *p_queue_dequeue(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  void *const value = p_queue_front(queue);
  queue->used_allocation -= get_overall_member_size(queue);
  queue->front_offset += sizeof(p_queue_member);
  return value;
}

static inline size_t get_members_extent(priority_queue *queue) {
  return p_queue_get_length(queue) + p_queue_get_capacity(queue);
}

priority_queue *p_queue_resize(priority_queue *queue, const size_t new_size) {
  size_t PADDING_BYTES = 0;

  if (queue->allocation < new_size) {
    PADDING_BYTES = calc_member_padding(new_size);
    printf("%lld", (byte *)p_queue_back(queue) - (byte *)queue + new_size);
    exit(0);
  }

  queue = realloc(queue, new_size + PADDING_BYTES);
  if (queue == NULL) return NULL;

  queue->allocation = new_size;

  queue->begin_values = queue + 1;
  queue->begin_members =
      (void *)((byte *)queue->begin_values +
               queue->value_size * get_members_extent(queue) + PADDING_BYTES);
  
  return queue;
}

int main(void) {
  const int data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  priority_queue *a = new_p_queue(data);
  a = p_queue_resize(a, a->allocation);

  void *item = p_queue_dequeue(a);
  while (item) {
    printf("%d ", *(int *)item);
    item = p_queue_dequeue(a);
  }
  return 0;
}
