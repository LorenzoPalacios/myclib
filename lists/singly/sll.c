#include "sll.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../stack/stack.h"

typedef unsigned char byte;

#define NULL_INDEX (SIZE_MAX)

// - STRUCTS -

// singly-linked list node (typedef in associated header file)
struct sl_node {
  size_t next_index;
};

// singly-linked list (typedef in associated header file)
struct sl_list {
  size_t head_index;
  size_t tail_index;
  size_t unalloc_region_index;
  size_t value_size;
  size_t allocation;
  stack *deleted_nodes;
};

// - INTERNAL -

static inline size_t data_allocation(const sl_list *const list) {
  return list->allocation - sizeof(sl_list);
}

static inline size_t node_value_pair_size(const sl_list *const list) {
  return sizeof(sl_node) + list->value_size;
}

static inline size_t active_length(const sl_list *const list) {
  return list->unalloc_region_index;
}

static inline size_t inactive_length(const sl_list *const list) {
  return sll_capacity(list) - active_length(list);
}

static inline size_t values_alloc(const sl_list *const list) {
  return sll_capacity(list) * list->value_size;
}

static inline size_t nodes_alloc(const sl_list *const list) {
  return sll_capacity(list) * sizeof(sl_node);
}

static inline byte *get_values(sl_list *const list) {
  return (byte *)(list + 1);
}

static inline size_t calc_padding_bytes(const size_t values_alloc) {
  const size_t SIZE_DIFF = values_alloc % alignof(sl_node);
  if (SIZE_DIFF == 0) return 0;
  const size_t PADDING_BYTES = alignof(sl_node) - SIZE_DIFF;
  return PADDING_BYTES;
}

static inline size_t padding_bytes(const sl_list *const list) {
  return calc_padding_bytes(values_alloc(list));
}

static inline sl_node *get_nodes(sl_list *const list) {
  byte *const VALUES_END = get_values(list) + values_alloc(list);
  return (void *)(VALUES_END + padding_bytes(list));
}

static inline size_t node_index(const sl_list *const list,
                                const sl_node *const node) {
  // This function calculates the start of the nodes region so that `list` can
  // retain its `const` qualifier. It is equivalent to an inline expansion of
  // `get_nodes()`.
  const sl_node *const NODES =
      (const void *)((const byte *)(list + 1) + values_alloc(list) +
                     padding_bytes(list));
  return (size_t)(node - NODES);
}

static inline sl_node *node_from_index(sl_list *const list,
                                       const size_t index) {
  if (index == NULL_INDEX) return NULL;
  return get_nodes(list) + index;
}

static inline sl_node *get_deleted_node(sl_list *const list) {
  stack *const deleted_nodes = list->deleted_nodes;
  const size_t cnt_deleted_nodes = deleted_nodes->length;

  sl_node *deleted_node = NULL;
  if (cnt_deleted_nodes > 0) {
    const size_t NODE_INDEX = *(size_t *)stack_pop_(deleted_nodes);
    deleted_node = node_from_index(list, NODE_INDEX);
  }
  return deleted_node;
}

static inline sl_node *get_open_node(sl_list *const list) {
  if (list->deleted_nodes->length != 0) return get_deleted_node(list);
  if (inactive_length(list) != 0)
    return node_from_index(list, list->unalloc_region_index++);
  return NULL;
}

// Helper function used by `sll_shrink()`.
// using naive method (for now)
static void fill_unallocated_gaps(sl_list *const list) {
  const size_t OVERALL_LENGTH = sll_capacity(list);
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
      byte *const deleted_value = sll_node_value(list, deleted_node);
      const size_t MOVE_SIZE = (size_t)(VALUES_END - deleted_value);
      memmove(deleted_value, deleted_value + VALUE_SIZE, MOVE_SIZE);
    }
    deleted_node = get_deleted_node(list);
  }
}

static inline void disconnect_node(sl_list *const list, sl_node *const node) {
  size_t prev_node_index = NULL_INDEX;
  {
    const size_t TARGET_INDEX = node_index(list, node);
    size_t cur_node_index = list->head_index;
    while (cur_node_index != TARGET_INDEX) {
      prev_node_index = cur_node_index;
      cur_node_index = node_from_index(list, cur_node_index)->next_index;
    }
  }
  sl_node *const prev_node = node_from_index(list, prev_node_index);
  prev_node->next_index = node->next_index;
}

// Helper function used by `sll_delete_node()`.
static inline bool register_deleted_node(sl_list *const list,
                                         const sl_node *const node) {
  const size_t NODE_INDEX = node_index(list, node);
  stack *const deleted_nodes = stack_push(list->deleted_nodes, &NODE_INDEX);
  if (deleted_nodes == NULL) return false;
  list->deleted_nodes = deleted_nodes;
  return true;
}

// - LIBRARY -

sl_list *sll_add_node(sl_list *list, const void *const value) {
  sl_node *new_node = get_open_node(list);
  if (new_node == NULL) {
    list = sll_expand(list);
    if (list == NULL) return NULL;
    new_node = get_open_node(list);
  }
  const size_t NEW_NODE_INDEX = node_index(list, new_node);
  memcpy(sll_node_value(list, new_node), value, list->value_size);
  if (list->tail_index != NULL_INDEX) {
    sl_node *const former_end_node = node_from_index(list, list->tail_index);
    former_end_node->next_index = NEW_NODE_INDEX;
  }
  if (list->head_index == NULL_INDEX) list->head_index = NEW_NODE_INDEX;
  list->tail_index = NEW_NODE_INDEX;

  new_node->next_index = NULL_INDEX;
  return list;
}

size_t sll_capacity(const sl_list *const list) {
  const size_t ALLOC = data_allocation(list);
  const size_t PAIR_SIZE = node_value_pair_size(list);
  return ALLOC / PAIR_SIZE;
}

void sll_delete(sl_list *const list) {
  stack_delete(list->deleted_nodes);
  free(list);
}

bool sll_delete_node(sl_list *const list, sl_node *const node) {
  if (!register_deleted_node(list, node)) return false;
  disconnect_node(list, node);
  if (active_length(list) == 0)
    list->head_index = list->tail_index = NULL_INDEX;
  return true;
}

sl_list *sll_expand(sl_list *const list) {
  const size_t CUR_CAPACITY = sll_capacity(list);
  if (CUR_CAPACITY == 0) return sll_resize(list, 1);
  sl_list *new_list = sll_resize(list, 2 * CUR_CAPACITY);
  if (new_list == NULL) new_list = sll_resize(list, CUR_CAPACITY + 1);
  return new_list;
}

sl_node *sll_head(sl_list *const list) {
  return node_from_index(list, list->head_index);
}

sl_list *sll_init(const size_t num_nodes, const size_t value_size) {
  const size_t VALUES_ALLOC = num_nodes * value_size;
  const size_t NODES_ALLOC = num_nodes * sizeof(sl_node);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  const size_t TOTAL_ALLOC =
      VALUES_ALLOC + NODES_ALLOC + PADDING_BYTES + sizeof(sl_list);

  sl_list *const list = malloc(TOTAL_ALLOC);
  if (list == NULL) return NULL;
  list->head_index = list->tail_index = NULL_INDEX;
  list->unalloc_region_index = 0;
  list->value_size = value_size;
  list->allocation = TOTAL_ALLOC;
  list->deleted_nodes = stack_empty_new(num_nodes / 4, sizeof(size_t));
  return list;
}

sl_node *sll_next_node(sl_list *const list, sl_node *const node) {
  return node_from_index(list, node->next_index);
}

sl_list *sll_new_(const void *const data, const size_t num_values,
                  const size_t value_size) {
  sl_list *const list = sll_init(num_values, value_size);
  if (list == NULL) return NULL;

  {
    sl_node *const NODES_MEM = get_nodes(list);
    for (size_t i = 0; i < num_values - 1; i++) NODES_MEM[i].next_index = i + 1;
    NODES_MEM[num_values - 1].next_index = NULL_INDEX;
  }
  memcpy(get_values(list), data, num_values * value_size);
  list->head_index = 0;
  list->tail_index = num_values - 1;
  list->unalloc_region_index = num_values;
  return list;
}

void *sll_node_value(sl_list *list, sl_node *const node) {
  const size_t VALUE_OFFSET = list->value_size * node_index(list, node);
  return get_values(list) + VALUE_OFFSET;
}

sl_list *sll_resize(sl_list *list, const size_t new_capacity) {
  const size_t VALUES_ALLOC = list->value_size * new_capacity;
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  const size_t DATA_ALLOC = node_value_pair_size(list) * new_capacity;
  const size_t NEW_ALLOC = DATA_ALLOC + PADDING_BYTES + sizeof(sl_list);
  {
    if (NEW_ALLOC == list->allocation) return list;
    sl_list *const new_list = realloc(list, NEW_ALLOC);
    if (list == NULL) return NULL;
    list = new_list;
  }
  const size_t SHIFT_SIZE = nodes_alloc(list);
  sl_node *const OLD_NODES_REGION = get_nodes(list);
  list->allocation = NEW_ALLOC;
  sl_node *const NEW_NODES_REGION = get_nodes(list);
  memmove(NEW_NODES_REGION, OLD_NODES_REGION, SHIFT_SIZE);
  return list;
}

sl_list *sll_shrink(sl_list *const list) {
  fill_unallocated_gaps(list);
  return sll_resize(list, active_length(list));
}

void sll_swap_values(sl_list *const list, sl_node *const node_1,
                     sl_node *const node_2) {
  const size_t VALUE_SIZE = list->value_size;
  byte *const value_1 = sll_node_value(list, node_1);
  byte *const value_2 = sll_node_value(list, node_2);
  for (size_t i = 0; i < VALUE_SIZE; i++) {
    const byte temp = value_1[i];
    value_1[i] = value_2[i];
    value_2[i] = temp;
  }
}

sl_node *sll_tail(sl_list *const list) {
  return node_from_index(list, list->tail_index);
}

void sll_traverse(sl_list *const list, sl_node *const from,
                  bool (*const operation)(sl_list *list, sl_node *node)) {
  sl_node *cur_node = from;
  while (cur_node != NULL && operation(list, cur_node))
    cur_node = sll_next_node(list, cur_node);
}
