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

static inline size_t calc_padding_bytes(const size_t alloc_size) {
  const size_t ALIGNMENT_DIFF = alloc_size % alignof(p_queue_member);
  return (ALIGNMENT_DIFF != 0) ? (alignof(p_queue_member) - ALIGNMENT_DIFF) : 0;
}

static inline size_t get_members_extent(const priority_queue *const queue) {
  return p_queue_get_length(queue) + p_queue_get_capacity(queue);
}

/*
 * The pointer to `queue` is not `const` because my formatter `(clang-format)`
 * makes this function look weird whenever the `const` qualifier is applied to
 * `queue`.
 */
static inline size_t get_member_value_pair_size(const priority_queue *queue) {
  return queue->value_size + sizeof(p_queue_member);
}

/*
 * This function structures the memory of `queue` according to its allocation.
 */
static void configure_queue_mem(priority_queue *const queue) {
  const size_t ALLOC = queue->allocation;
  const size_t VALUE_SIZE = queue->value_size;
  const size_t MEMBER_VALUE_SIZE = get_member_value_pair_size(queue);
  const size_t NUM_MEMBERS = ALLOC / MEMBER_VALUE_SIZE;

  const size_t VALUE_ALLOC = NUM_MEMBERS * VALUE_SIZE;
  const size_t PADDING_BYTES = calc_padding_bytes(VALUE_ALLOC);

  /* The values will be stored directly after the queue header. */
  queue->begin_values = queue + 1;
  const byte *const end_values =
      (byte *)queue->begin_values + NUM_MEMBERS * VALUE_SIZE;

  /*
   * The members will be stored directly after the values, which may require
   * padding to avoid misalignment.
   */
  queue->begin_members = (void *)(end_values + PADDING_BYTES);
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

  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);

  const size_t TOTAL_ALLOC =
      MEMBERS_ALLOC + VALUES_ALLOC + PADDING_BYTES + sizeof(priority_queue);
  priority_queue *const queue = malloc(TOTAL_ALLOC);
  if (queue == NULL) return NULL;
  queue->member_front_index = 0;
  queue->member_back_index = 0;
  queue->value_size = value_size;
  queue->allocation = TOTAL_ALLOC;

  configure_queue_mem(queue);

  return queue;
}

static inline void *get_member_value(priority_queue *const queue,
                                     const p_queue_member *const member) {
  return (byte *)queue->begin_values + member->value_access_offset;
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
  p_queue_member *const non_reserve_members =
      queue->begin_members + RESERVED_MEMBERS;

  for (size_t i = 0; i < num_values; i++) {
    p_queue_member *const cur_member = non_reserve_members + i;
    const size_t val_offset = i * value_size;
    cur_member->priority = MEDIUM;
    cur_member->value_access_offset = val_offset;

    void *const cur_member_val = get_member_value(queue, cur_member);
    memcpy(cur_member_val, (byte *)data + i * value_size, value_size);
  }
  queue->member_front_index = non_reserve_members - queue->begin_members;
  queue->member_back_index = queue->member_front_index + num_values;

  return queue;
}

size_t p_queue_get_used_alloc(const priority_queue *const queue) {
  const size_t ALLOC = queue->allocation;
  const size_t MEMBER_SIZE = get_member_value_pair_size(queue);
  const size_t LENGTH = p_queue_get_length(queue);
  return ALLOC - LENGTH * MEMBER_SIZE;
}

size_t p_queue_get_length(const priority_queue *const queue) {
  return queue->member_back_index - queue->member_front_index;
}

size_t p_queue_get_capacity(const priority_queue *const queue) {
  const size_t UNUSED_ALLOC = queue->allocation - p_queue_get_used_alloc(queue);
  const size_t TOTAL_MEMBER_SIZE = queue->value_size + sizeof(p_queue_member);
  return UNUSED_ALLOC / TOTAL_MEMBER_SIZE;
}

void *p_queue_front(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  return get_member_value(queue,
                          queue->begin_members + queue->member_front_index);
}

void *p_queue_back(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  return get_member_value(queue,
                          queue->begin_members + queue->member_back_index);
}

void *p_queue_dequeue(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  void *const value = p_queue_front(queue);
  printf("front: %zu\n", queue->member_front_index);
  queue->member_front_index++;
  return value;
}

priority_queue *p_queue_resize(priority_queue *queue, size_t new_size) {
  if (new_size == queue->allocation) return queue;
  if (new_size < sizeof(priority_queue)) new_size += sizeof(priority_queue);

  const double size_ratio = (double)new_size / queue->allocation;

  const size_t NEW_NUM_MEMBERS = get_members_extent(queue) * size_ratio;
  const size_t NEW_VALUES_ALLOC = NEW_NUM_MEMBERS * queue->value_size;
  const size_t NEW_PADDING_BYTES = calc_padding_bytes(NEW_VALUES_ALLOC);

  queue = realloc(queue, new_size + NEW_PADDING_BYTES);
  if (queue == NULL) return NULL;

  queue->allocation = new_size;

  configure_queue_mem(queue);

  return queue;
}

int main(void) {
  const int data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  priority_queue *a = new_p_queue(data);
  a = p_queue_resize(a, a->allocation * 1.5);
  void *item = p_queue_dequeue(a);
  while (item) {
    printf("%d ", *(int *)item);
    item = p_queue_dequeue(a);
  }

  return 0;
}
