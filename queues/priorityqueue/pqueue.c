#include "pqueue.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

static inline size_t calc_padding_bytes(const size_t alloc_size) {
  const size_t ALIGNMENT_DIFF = alloc_size % alignof(p_queue_member);
  return (ALIGNMENT_DIFF != 0) ? (alignof(p_queue_member) - ALIGNMENT_DIFF) : 0;
}

static priority_queue *instantiate_p_queue(const size_t num_members,
                                    const size_t value_size,
                                    const size_t num_reserve_members) {
  const size_t TOTAL_MEMBERS = num_members + num_reserve_members;

  const size_t VALUES_ALLOC = TOTAL_MEMBERS * value_size;
  const size_t MEMBERS_ALLOC = TOTAL_MEMBERS * sizeof(p_queue_member);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);

  const size_t TOTAL_ALLOC =
      MEMBERS_ALLOC + VALUES_ALLOC + PADDING_BYTES + sizeof(priority_queue);
  priority_queue *const queue = malloc(TOTAL_ALLOC);
  if (queue == NULL) return NULL;
  queue->front_index = 0;
  queue->back_index = 0;
  queue->value_size = value_size;
  /*
   * It's generally more helpful to keep track of the memory used by queue
   * values and members than it is to keep track of the memory every component
   * (such as the queue header) uses.
   */
  queue->data_allocation = VALUES_ALLOC + MEMBERS_ALLOC;

  return queue;
}

/*
 * The pointer to `queue` is not `const` because my formatter `(clang-format)`
 * makes this function look weird whenever the `const` qualifier is applied to
 * `queue`.
 */
static inline size_t get_member_value_pair_size(const priority_queue *queue) {
  return queue->value_size + sizeof(p_queue_member);
}

static inline size_t get_num_members(const priority_queue *const queue) {
  const size_t MEMBER_AND_VALUE_SIZE = get_member_value_pair_size(queue);
  const size_t NUM_MEMBERS = queue->data_allocation / MEMBER_AND_VALUE_SIZE;
  return NUM_MEMBERS;
}

static inline size_t get_members_alloc(const priority_queue *const queue) {
  const size_t NUM_MEMBERS = get_num_members(queue);
  const size_t MEMBERS_ALLOC = NUM_MEMBERS * sizeof(p_queue_member);
  return MEMBERS_ALLOC;
}

static inline size_t get_values_alloc(const priority_queue *const queue) {
  const size_t NUM_MEMBERS = get_num_members(queue);
  const size_t VALUES_ALLOC = NUM_MEMBERS * queue->value_size;
  return VALUES_ALLOC;
}

static inline byte *get_values_region(priority_queue *const queue) {
  return (void *)(queue + 1);
}

static inline size_t current_padding_bytes(priority_queue *const queue) {
  const size_t MEMBER_AND_VALUE_SIZE = get_member_value_pair_size(queue);
  const size_t NUM_MEMBERS = queue->data_allocation / MEMBER_AND_VALUE_SIZE;
  const size_t VALUES_ALLOC = NUM_MEMBERS * queue->value_size;
  return calc_padding_bytes(VALUES_ALLOC);
}

static inline p_queue_member *get_members_region(priority_queue *const queue) {
  const size_t VALUES_EXTENT = get_values_alloc(queue);
  const size_t PADDING_BYTES = current_padding_bytes(queue);
  byte *const VALUES_END = get_values_region(queue) + VALUES_EXTENT;
  p_queue_member *MEMBERS_BEGIN = (void *)(VALUES_END + PADDING_BYTES);
  return MEMBERS_BEGIN;
}

static inline void *get_member_value(priority_queue *const queue,
                                     const p_queue_member *const member) {
  const size_t MEMBER_INDEX = member - get_members_region(queue);
  return get_values_region(queue) + MEMBER_INDEX * queue->value_size;
}

priority_queue *_new_p_queue(const void *const data, const size_t num_values,
                             const size_t value_size) {
  /*
   * This defines a percentage of extraneous members to be added to facilitate
   * the efficient enqueuing of members with high priority.
   */
  static const double PERCENT_RESERVED_MEMBERS = (0.25);

  const size_t RESERVED_MEMBERS = PERCENT_RESERVED_MEMBERS * num_values;
  priority_queue *const queue =
      instantiate_p_queue(num_values, value_size, RESERVED_MEMBERS);

  p_queue_member *const members = get_members_region(queue);

  /*
   * Reserved members (which are present at the front and back of the queue)
   * should not be written to during initialization, so we skip over them.
   */
  size_t data_i = 0;
  size_t members_i = queue->front_index = RESERVED_MEMBERS / 2;
  while (data_i < num_values) {
    p_queue_member *const cur_member = members + members_i;
    cur_member->priority = MEDIUM;
    void *const cur_member_val = get_member_value(queue, cur_member);
    memcpy(cur_member_val, (byte *)data + data_i * value_size, value_size);
    data_i++;
    members_i++;
  }
  queue->back_index = queue->front_index + num_values;

  return queue;
}

size_t p_queue_get_used_alloc(const priority_queue *const queue) {
  const size_t MEMBER_VALUE_SIZE = get_member_value_pair_size(queue);
  const size_t LENGTH = p_queue_get_length(queue);
  return LENGTH * MEMBER_VALUE_SIZE;
}

size_t p_queue_get_length(const priority_queue *const queue) {
  return queue->back_index - queue->front_index;
}

size_t p_queue_get_capacity(const priority_queue *const queue) {
  const size_t UNUSED_ALLOC =
      queue->data_allocation - p_queue_get_used_alloc(queue);
  const size_t TOTAL_MEMBER_SIZE = queue->value_size + sizeof(p_queue_member);
  return UNUSED_ALLOC / TOTAL_MEMBER_SIZE;
}

void *p_queue_front(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  p_queue_member *const members = get_members_region(queue);
  p_queue_member *const front = members + queue->front_index;
  return get_member_value(queue, front);
}

void *p_queue_back(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  p_queue_member *const members = get_members_region(queue);
  p_queue_member *const back = members + queue->back_index;
  return get_member_value(queue, back);
}

void *p_queue_dequeue(priority_queue *const queue) {
  if (p_queue_get_length(queue) == 0) return NULL;
  void *const value = p_queue_front(queue);
  queue->front_index++;
  return value;
}

/*
 * Helper function used by `p_queue_resize()` to ensure all new sizes are
 * efficient and comply with alignment.
 */
static inline size_t align_new_size(const priority_queue *const queue,
                                    size_t new_size) {
  const size_t MEMBER_VALUE_SIZE = get_member_value_pair_size(queue);
  const size_t SIZE_DIFF = new_size % MEMBER_VALUE_SIZE;
  if (SIZE_DIFF != 0) new_size += MEMBER_VALUE_SIZE - SIZE_DIFF;
  return new_size;
}

priority_queue *p_queue_resize(priority_queue *queue, const size_t new_size) {
  if (new_size == queue->data_allocation) return queue;
  if (new_size <= sizeof(priority_queue))
    return realloc(queue, sizeof(priority_queue));

  const size_t ALIGNED_SIZE = align_new_size(queue, new_size);

  const double SIZE_RATIO = (double)ALIGNED_SIZE / queue->data_allocation;

  const size_t VALUE_SIZE = queue->value_size;
  const size_t OLD_NUM_MEMBERS = p_queue_get_length(queue);
  const size_t NEW_NUM_MEMBERS = SIZE_RATIO * get_num_members(queue);
  const size_t NEW_VALUES_ALLOC = NEW_NUM_MEMBERS * VALUE_SIZE;
  const size_t NEW_PADDING_BYTES = calc_padding_bytes(NEW_VALUES_ALLOC);

  queue = realloc(queue, ALIGNED_SIZE + NEW_PADDING_BYTES);
  if (queue == NULL) return NULL;

  if (ALIGNED_SIZE > queue->data_allocation) {
    const size_t SHIFT_COUNT = (NEW_NUM_MEMBERS - OLD_NUM_MEMBERS) / 2;
    {
      byte *const OLD_VALUES_START =
          get_values_region(queue) + queue->front_index * VALUE_SIZE;
      byte *const NEW_VALUES_START =
          OLD_VALUES_START + SHIFT_COUNT * VALUE_SIZE;
      memmove(NEW_VALUES_START, OLD_VALUES_START, OLD_NUM_MEMBERS * VALUE_SIZE);
    }

    {
      p_queue_member *const OLD_MEMBERS_START =
          get_members_region(queue) + queue->front_index;
      p_queue_member *const NEW_MEMBERS_START = OLD_MEMBERS_START + SHIFT_COUNT;
      memmove(NEW_MEMBERS_START, OLD_MEMBERS_START,
              OLD_NUM_MEMBERS * sizeof(p_queue_member));
    }
    queue->front_index += SHIFT_COUNT;
    queue->back_index += SHIFT_COUNT;
  }

  queue->data_allocation = new_size;

  return queue;
}

void _delete_p_queue(priority_queue **const queue) {
  free(*queue);
  *queue = NULL;
}

void _delete_p_queue_s(priority_queue **const queue) {
  const size_t TOTAL_ALLOC = (*queue)->data_allocation + sizeof(priority_queue);
  memset(*queue, 0, TOTAL_ALLOC);
  _delete_p_queue(queue);
}

priority_queue *p_queue_expand(priority_queue *queue) {
  queue = p_queue_resize(queue, queue->data_allocation * 2);
  /* 
   * If the first resize fails, assume it encountered an out-of-memory error, so
   * instead request a smaller expansion for one extra member-value pair.
   */
  if (queue == NULL) {
    const size_t MEMBER_VALUE_SIZE = get_member_value_pair_size(queue);
    queue = p_queue_resize(queue, queue->data_allocation + MEMBER_VALUE_SIZE);
  }
  return queue;
}

int main(void) {
  const int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  priority_queue *a = new_p_queue(data);
  a = p_queue_resize(a, a->data_allocation);
  const size_t queue_length = p_queue_get_length(a);
  for (size_t i = 0; i < queue_length; i++) {
    const void *const item = p_queue_dequeue(a);
    printf("value: %d\n", *(int *)item);
  }
  delete_p_queue(a);

  return 0;
}
