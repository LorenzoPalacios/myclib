#include "sll.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../stack/stack.h"

typedef unsigned char byte;

/* - STRUCTS - */

/* singly-linked list node (typedef in associated header file) */
struct sl_node {
  size_t next_index;
};

/*
 *
 */
struct sl_list {
  stack deleted_nodes;
  void *list_contents;
  size_t head_index;
  size_t tail_index;
  size_t nodes_extent;
  size_t value_size;
  size_t capacity;
};

/* - INTERNAL - */

static inline size_t node_value_pair_size(const sl_list *const list) {
  return sizeof(sl_node) + list->value_size;
}

static inline size_t active_length(const sl_list *const list) {
  return list->nodes_extent - list->deleted_nodes.length;
}

static inline size_t inactive_length(const sl_list *const list) {
  return list->capacity - list->nodes_extent;
}

static inline size_t values_alloc(const sl_list *const list) {
  return list->capacity * list->value_size;
}

static inline size_t nodes_alloc(const sl_list *const list) {
  return list->capacity * sizeof(sl_node);
}

static inline byte *get_values(const sl_list *const list) {
  return list->list_contents;
}

/*
 * The two instances of `sizeof(sl_node)` were once `alignof(sl_node)`, but for
 * the sake of backwards compatability with C standards prior to C11, they
 * have been changed to `sizeof(sl_node)`. So long as `sl_node` only contains
 * `size_t` members, this will work.
 */
static inline size_t calc_padding_bytes(const size_t values_alloc) {
  const size_t SIZE_DIFF = values_alloc % sizeof(sl_node);
  return SIZE_DIFF == 0 ? 0 : sizeof(sl_node) - SIZE_DIFF;
}

static inline size_t padding_bytes(const sl_list *const list) {
  return calc_padding_bytes(values_alloc(list));
}

static inline sl_node *get_nodes(const sl_list *const list) {
  byte *const VALUES_END = get_values(list) + values_alloc(list);
  return (void *)(VALUES_END + padding_bytes(list));
}

static inline size_t node_index(const sl_list *const list,
                                const sl_node *const node) {
  /*
   * This function calculates the start of the nodes region so that `list` can
   * retain its `const` qualifier. It is equivalent to an inline expansion of
   * `get_nodes()`.
   */
  const sl_node *const NODES =
      (const void *)((const byte *)(list + 1) + values_alloc(list) +
                     padding_bytes(list));
  return (size_t)(node - NODES);
}

static inline sl_node *node_from_index(const sl_list *const list,
                                       const size_t index) {
  return get_nodes(list) + index;
}

static inline sl_node *get_deleted_node(sl_list *const list) {
  stack *const deleted_nodes = &list->deleted_nodes;
  const size_t cnt_deleted_nodes = deleted_nodes->length;
  return (cnt_deleted_nodes > 0)
             ? node_from_index(list, *(size_t *)stack_pop_(deleted_nodes))
             : NULL;
}

/* Helper function used by sll_add_node(). */
static inline sl_node *get_open_node(sl_list *const list) {
  if (list->deleted_nodes.length != 0) return get_deleted_node(list);
  if (inactive_length(list) != 0)
    return node_from_index(list, list->nodes_extent++);
  return NULL;
}

/* Helper function used by `sll_shrink()`. */
static void fill_unallocated_gaps(sl_list *const list) {
  const size_t OVERALL_LENGTH = list->capacity;
  const size_t VALUE_SIZE = list->value_size;
  byte *const VALUES_END = get_values(list) + (OVERALL_LENGTH * VALUE_SIZE);
  byte *const NODES_END = (byte *)(get_nodes(list) + OVERALL_LENGTH);

  sl_node *deleted_node = get_deleted_node(list);
  while (deleted_node != NULL) {
    {
      const size_t MOVE_SIZE = (size_t)(NODES_END - (byte *)deleted_node);
      memmove(deleted_node, deleted_node + 1, MOVE_SIZE);
    }
    {
      byte *const deleted_value = sll_node_value_(list, deleted_node);
      const size_t MOVE_SIZE = (size_t)(VALUES_END - deleted_value);
      memmove(deleted_value, deleted_value + VALUE_SIZE, MOVE_SIZE);
    }
    deleted_node = get_deleted_node(list);
  }
}

static inline void disconnect_node(const sl_list *const list,
                                   const sl_node *const node) {
  sl_node *const prev_node = sll_prev_node_(list, node);
  prev_node->next_index = node->next_index;
}

/* Helper function used by `sll_delete_node()`. */
static inline bool register_deleted_node(sl_list *const list,
                                         const sl_node *const node) {
  const size_t NODE_INDEX = node_index(list, node);
  return stack_push(list->deleted_nodes, &NODE_INDEX);
}

/* - LIBRARY - */

bool sll_add_node_(sl_list *const list, const void *const value) {
  sl_node *new_node = get_open_node(list);
  size_t NEW_NODE_INDEX = node_index(list, new_node);
  if (new_node == NULL) {
    if (!sll_expand_(list)) return false;
    new_node = get_open_node(list);
  }

  memcpy(sll_node_value_(list, new_node), value, list->value_size);
  if (active_length(list) == 1)
    list->head_index = NEW_NODE_INDEX;
  list->tail_index = NEW_NODE_INDEX;
  return true;
}

void sll_delete_(const sl_list *const list) {
  stack_delete(list->deleted_nodes);
  free(list->list_contents);
}

bool sll_delete_node_(sl_list *const list, const sl_node *const node) {
  if (!register_deleted_node(list, node)) return false;
  disconnect_node(list, node);
  return true;
}

bool sll_expand_(sl_list *const list) {
  const size_t CUR_CAPACITY = list->capacity;
  const bool SUCCESS = sll_resize_(list, 2 * CUR_CAPACITY);
  if (!SUCCESS) return sll_resize_(list, CUR_CAPACITY + 1);
  return true;
}

sl_node *sll_head_(const sl_list *const list) {
  return node_from_index(list, list->head_index);
}

sl_list sll_init_(const size_t value_size, const size_t num_nodes) {
  const size_t VALUES_ALLOC = num_nodes * value_size;
  const size_t NODES_ALLOC = num_nodes * sizeof(sl_node);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  const size_t TOTAL_ALLOC =
      VALUES_ALLOC + NODES_ALLOC + PADDING_BYTES + sizeof(sl_list);

  sl_list list;
  list.list_contents = malloc(TOTAL_ALLOC);
  /* head_index and tail_index are deliberately left undefined. */
  list.nodes_extent = 0;
  list.value_size = value_size;
  list.capacity = num_nodes;
  list.deleted_nodes = stack_init(size_t, num_nodes / 4);
  return list;
}

sl_list sll_new_(const void *const data, const size_t num_values,
                 const size_t value_size) {
  sl_list list = sll_init_(value_size, num_values);

  {
    sl_node *const nodes = get_nodes(&list);
    size_t idx = 0;
    for (; idx < num_values - 1; idx++) nodes[idx].next_index = idx + 1;
  }
  memcpy(get_values(&list), data, num_values * value_size);
  list.head_index = 0;
  list.tail_index = num_values - 1;
  list.nodes_extent = num_values;
  return list;
}

sl_node *sll_next_node_(const sl_list *const list, const sl_node *const node) {
  return node_from_index(list, node->next_index);
}

void *sll_node_value_(const sl_list *const list, const sl_node *const node) {
  const size_t VALUE_OFFSET = list->value_size * node_index(list, node);
  return get_values(list) + VALUE_OFFSET;
}

sl_node *sll_prev_node_(const sl_list *const list, const sl_node *const node) {
  sl_node *prev_node = NULL;
  {
    sl_node *cur_node = sll_head_(list);
    while (cur_node != node) {
      prev_node = cur_node;
      cur_node = sll_next_node_(list, cur_node);
    }
  }
  return prev_node;
}

bool sll_resize_(sl_list *const list, const size_t new_capacity) {
  if (new_capacity != list->capacity) {
    const size_t SHIFT_SIZE = nodes_alloc(list);
    sl_node *const OLD_NODES_REGION = get_nodes(list);
    {
      const size_t VALUES_ALLOC = list->value_size * new_capacity;
      const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
      const size_t DATA_ALLOC = node_value_pair_size(list) * new_capacity;
      const size_t NEW_ALLOC = DATA_ALLOC + PADDING_BYTES;
      void *const new_list_contents = realloc(list->list_contents, NEW_ALLOC);
      if (list == NULL) return false;
      list->list_contents = new_list_contents;
    }
    list->capacity = new_capacity;
    {
      sl_node *const NEW_NODES_REGION = get_nodes(list);
      memmove(NEW_NODES_REGION, OLD_NODES_REGION, SHIFT_SIZE);
    }
    return true;
  }
  return false;
}

bool sll_shrink_(sl_list *const list) {
  fill_unallocated_gaps(list);
  return sll_resize_(list, active_length(list));
}

void sll_swap_(const sl_list *const list, sl_node *const node_1,
               sl_node *const node_2) {
  const size_t VALUE_SIZE = list->value_size;
  byte *const value_1 = sll_node_value_(list, node_1);
  byte *const value_2 = sll_node_value_(list, node_2);
  size_t byte_i = 0;
  for (; byte_i < VALUE_SIZE; byte_i++) {
    const byte temp = value_1[byte_i];
    value_1[byte_i] = value_2[byte_i];
    value_2[byte_i] = temp;
  }
}

sl_node *sll_tail_(const sl_list *const list) {
  return node_from_index(list, list->tail_index);
}

void sll_traverse_(sl_list *const list, sl_node *const from,
                   bool (*const operation)(sl_list *list, sl_node *node)) {
  sl_node *cur_node = from;
  while (cur_node != NULL && operation(list, cur_node))
    cur_node = sll_next_node_(list, cur_node);
}
