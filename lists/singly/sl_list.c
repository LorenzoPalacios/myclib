#include "sl_list.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

static inline size_t node_value_pair_size(const sl_list *const list) {
  const size_t VALUE_SIZE = list->value_size;
  return sizeof(sl_node) + VALUE_SIZE;
}

static inline size_t sl_list_overall_length(const sl_list *const list) {
  const size_t ALLOC = list->data_allocation;
  const size_t PAIR_SIZE = node_value_pair_size(list);
  return ALLOC / PAIR_SIZE;
}

static inline size_t sl_list_unused_length(const sl_list *const list) {
  const size_t OVERALL_LENGTH = sl_list_overall_length(list);
  const size_t USED_LENGTH = list->length;
  const size_t UNUSED_LENGTH = OVERALL_LENGTH - USED_LENGTH;
  return UNUSED_LENGTH;
}

static inline size_t get_values_alloc(const sl_list *const list) {
  const size_t LENGTH = sl_list_overall_length(list);
  const size_t VALUE_SIZE = list->value_size;
  const size_t VALUES_ALLOC = LENGTH * VALUE_SIZE;
  return VALUES_ALLOC;
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

static inline size_t get_padding_bytes(const sl_list *const list) {
  const size_t VALUES_ALLOC = get_values_alloc(list);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  return PADDING_BYTES;
}

static inline sl_node *get_nodes(sl_list *const list) {
  const size_t VALUES_ALLOC = get_values_alloc(list);
  const size_t PADDING_BYTES = get_padding_bytes(list);
  byte *const VALUES = get_values(list);
  byte *const VALUES_END = VALUES + VALUES_ALLOC;
  sl_node *const NODES = (void *)(VALUES_END + PADDING_BYTES);
  return NODES;
}

static inline size_t get_node_index(sl_list *const list,
                                    const sl_node *const node) {
  const sl_node *const NODES = get_nodes(list);
  const size_t NODE_INDEX = node - NODES;
  return NODE_INDEX;
}

void *sl_list_get_node_value(sl_list *const list, const sl_node *const node) {
  const size_t VALUE_SIZE = list->value_size;
  const size_t NODE_INDEX = get_node_index(list, node);
  const size_t NODE_VALUE_OFFSET = VALUE_SIZE * NODE_INDEX;
  byte *const VALUES = get_values(list);
  byte *const NODE_VALUE = VALUES + NODE_VALUE_OFFSET;
  return NODE_VALUE;
}

sl_list *sl_list_init(const size_t num_nodes, const size_t value_size) {
  const size_t VALUES_ALLOC = num_nodes * value_size;
  const size_t NODES_ALLOC = num_nodes * sizeof(sl_node);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  const size_t HEADER_ALLOC = sizeof(sl_list);
  const size_t TOTAL_ALLOC =
      HEADER_ALLOC + VALUES_ALLOC + NODES_ALLOC + PADDING_BYTES;

  sl_list *const list = malloc(TOTAL_ALLOC);
  if (list == NULL) return NULL;

  list->start_index = list->end_index = 0;
  list->deleted_nodes = new_empty_stack(num_nodes / 4, sizeof(size_t));
  list->length = 0;
  list->data_allocation = VALUES_ALLOC + NODES_ALLOC;
  list->value_size = value_size;
  return list;
}

static inline sl_node *get_node_from_index(sl_list *const list,
                                           const size_t index) {
  sl_node *const NODES = get_nodes(list);
  return NODES + index;
}

static inline sl_node *sl_list_get_next_node(sl_list *const list,
                                             sl_node *const node) {
  const size_t NODE_INDEX = get_node_index(list, node);
  if (NODE_INDEX == list->end_index || list->length == 0) return NULL;
  sl_node *const NODES = get_nodes(list);
  sl_node *const NEXT_NODE = NODES + node->next_node_index;
  return NEXT_NODE;
}

void *sl_list_traverse(sl_list *const list,
                       void *(*const op)(sl_list *list, sl_node *node),
                       bool (*const exit_cond)(sl_list *list, sl_node *node)) {
  void *latest_op_value = NULL;
  sl_node *cur_node = get_node_from_index(list, list->start_index);
  while (cur_node != NULL) {
    if (op != NULL) latest_op_value = op(list, cur_node);
    if (exit_cond != NULL)
      if (!exit_cond(list, cur_node)) break;
    cur_node = sl_list_get_next_node(list, cur_node);
  }
  return latest_op_value;
}

sl_list *_sl_list_create(const void *const data, const size_t num_elems,
                         const size_t elem_size) {
  sl_list *const list = sl_list_init(num_elems, elem_size);
  if (list == NULL) return NULL;

  byte *const VALUES_MEM = get_values(list);
  sl_node *const NODES_MEM = get_nodes(list);

  sl_node *cur_node = NODES_MEM;
  for (size_t i = 0; i < num_elems; i++) {
    const void *const cur_elem = (byte *)data + i * elem_size;
    void *const cur_value = VALUES_MEM + i * elem_size;
    memcpy(cur_value, cur_elem, elem_size);
    cur_node->next_node_index = i + 1;
    /*
     * Incrementing to the next node since `cur_node` is a copy of the pointer
     * to `NODES_MEM`.
     */
    cur_node++;
  }
  list->start_index = 0;
  list->end_index = num_elems - 1;
  list->length = num_elems;

  return list;
}

static inline size_t sanitize_new_size(const sl_list *const list,
                                       const size_t new_size) {
  const size_t PAIR_SIZE = node_value_pair_size(list);
  const size_t USED_LENGTH = list->length;
  const size_t SIZE_LOWER_BOUND = PAIR_SIZE * USED_LENGTH;
  if (new_size < SIZE_LOWER_BOUND) return SIZE_LOWER_BOUND;
  /*
   * This ensures the new size will always be a multiple of `PAIR_SIZE`, which
   * makes getting the padding byte allocation easier in later functions.
   */
  const size_t SANITIZED_NEW_SIZE = new_size / PAIR_SIZE * PAIR_SIZE;
  return SANITIZED_NEW_SIZE;
}

static inline size_t align_new_size(const sl_list *const list,
                                    const size_t new_size) {
  const size_t PAIR_SIZE = node_value_pair_size(list);
  const size_t NEW_LIST_LENGTH = new_size / PAIR_SIZE;

  const size_t VALUE_SIZE = list->value_size;
  const size_t NEW_VALUES_ALLOC = NEW_LIST_LENGTH * VALUE_SIZE;

  const size_t PADDING_BYTES = calc_padding_bytes(NEW_VALUES_ALLOC);
  const size_t ALIGNED_SIZE = NEW_LIST_LENGTH * PAIR_SIZE + PADDING_BYTES;
  return ALIGNED_SIZE;
}

sl_list *sl_list_resize(sl_list *list, const size_t new_size) {
  const size_t BOUNDED_NEW_SIZE = sanitize_new_size(list, new_size);
  const size_t ALIGNED_ALLOC = align_new_size(list, BOUNDED_NEW_SIZE);
  const size_t PADDING_BYTES = ALIGNED_ALLOC - BOUNDED_NEW_SIZE;
  const size_t DATA_ALLOC = ALIGNED_ALLOC - PADDING_BYTES;
  {
    if (DATA_ALLOC == list->data_allocation) return list;
    const size_t HEADER_SIZE = sizeof(sl_list);
    const size_t TOTAL_NEW_ALLOC = HEADER_SIZE + ALIGNED_ALLOC;
    list = realloc(list, TOTAL_NEW_ALLOC);
    if (list == NULL) return NULL;
  }
  sl_node *const OLD_NODES_REGION = get_nodes(list);

  list->data_allocation = DATA_ALLOC;

  sl_node *const NEW_NODES_REGION = get_nodes(list);
  const size_t LENGTH = list->length;
  const size_t SHIFT_SIZE = LENGTH * sizeof(sl_node);
  if (new_size == 360) {
    printf("diff: %lld", NEW_NODES_REGION - OLD_NODES_REGION);
    exit(0);
  }
  memmove(NEW_NODES_REGION, OLD_NODES_REGION, SHIFT_SIZE);

  return list;
}

sl_list *sl_list_expand(sl_list *const list) {
  const size_t ORIGINAL_SIZE = list->data_allocation;
  sl_list *new_list = sl_list_resize(list, ORIGINAL_SIZE * 2);
  if (new_list == NULL) {
    const size_t PAIR_SIZE = node_value_pair_size(list);
    new_list = sl_list_resize(list, ORIGINAL_SIZE + PAIR_SIZE);
  } else if (ORIGINAL_SIZE == 0) {
    const size_t PAIR_SIZE = node_value_pair_size(new_list);
    new_list = sl_list_resize(new_list, ORIGINAL_SIZE + PAIR_SIZE);
  }
  return new_list;
}
/*
 * Helper function for `sl_list_delete_node()`.
 *
 * \note The target node can be set by passing `NULL` for `list` and the
 * target node as `cur_node` from outside `sl_list_traverse()`, which cannot
 * pass a `NULL` list to this function as `sl_list_delete_node()` supplies a
 * valid list argument.
 */
static bool end_reconnection(sl_list *const list, sl_node *const cur_node) {
  static const sl_node *target_node = NULL;
  if (list == NULL) target_node = cur_node;
  return cur_node != target_node;
}

/*
 * Helper function for `sl_list_delete_node()`.
 *
 * This function reassigns a list's `end` node to the node just before `end`.
 * The return value of this function matters only to `sl_list_traverse()`.
 *
 * \note The target node can be set by passing `NULL` for `list` and the
 * target node as `cur_node` from outside `sl_list_traverse()`, which cannot
 * pass a `NULL` list to this function as `sl_list_delete_node()` supplies a
 * valid list argument.
 */
static void *reconnect_list(sl_list *const list, sl_node *const cur_node) {
  static sl_node *prev_node = NULL;
  static sl_node *target_node = NULL;

  if (list == NULL) {
    target_node = cur_node;
    return NULL;
  }

  if (target_node != NULL && cur_node == target_node) {
    if (prev_node == NULL) {
      sl_node *const start_node = get_node_from_index(list, list->start_index);
      list->start_index = start_node->next_node_index;
    } else {
      prev_node->next_node_index = cur_node->next_node_index;
    }
  } else {
    prev_node = cur_node;
  }

  return NULL;
}

bool sl_list_delete_node(sl_list *const list, sl_node *const node) {
  if (node == NULL) return false;

  {
    const size_t NODE_INDEX = get_node_index(list, node);
    stack *const deleted_nodes = stack_push(list->deleted_nodes, &NODE_INDEX);
    if (deleted_nodes == NULL) return false;
    list->deleted_nodes = deleted_nodes;
  }
  /* Initialize the target nodes for the helper functions. */
  reconnect_list(NULL, node);
  end_reconnection(NULL, node);

  sl_list_traverse(list, reconnect_list, end_reconnection);
  if (list->length == 1) list->start_index = list->end_index = 0;
  list->length--;
  return true;
}

static inline sl_node *get_unused_node(sl_list *const list) {
  sl_node *unused_node = NULL;
  const size_t UNUSED_LENGTH = sl_list_unused_length(list);
  if (UNUSED_LENGTH == 0) return NULL;
  sl_node *const nodes = get_nodes(list);
  const size_t num_used_nodes = list->length;
  unused_node = nodes + num_used_nodes;
  return unused_node;
}

static inline sl_node *get_deleted_node(sl_list *const list) {
  stack *const deleted_nodes = list->deleted_nodes;
  const size_t num_deleted_nodes = deleted_nodes->length;

  sl_node *deleted_node = NULL;
  if (num_deleted_nodes > 0) {
    const size_t NODE_INDEX = *(size_t *)stack_pop(deleted_nodes);
    deleted_node = get_node_from_index(list, NODE_INDEX);
  }
  return deleted_node;
}

static sl_node *get_open_node(sl_list *const list) {
  sl_node *open_node = get_deleted_node(list);
  if (open_node == NULL) open_node = get_unused_node(list);
  return open_node;
}

sl_list *sl_list_add_node(sl_list *list, const void *const value) {
  {
    const stack *const deleted_nodes = list->deleted_nodes;
    const size_t num_deleted_nodes = deleted_nodes->length;
    const size_t num_unused_nodes = sl_list_unused_length(list);
    if (num_deleted_nodes == 0 && num_unused_nodes == 0)
      list = sl_list_expand(list);
    if (list == NULL) return NULL;
  }

  sl_node *const new_node = get_open_node(list);
  if (new_node == NULL) return NULL;
  sl_node *const nodes = get_nodes(list);
  const size_t NEW_NODE_INDEX = new_node - nodes;
  {
    const size_t VALUE_SIZE = list->value_size;
    byte *const node_value = sl_list_get_node_value(list, new_node);
    memcpy(node_value, value, VALUE_SIZE);
  }
  sl_node *const former_end_node = get_node_from_index(list, list->end_index);
  former_end_node->next_node_index = NEW_NODE_INDEX;
  new_node->next_node_index = NEW_NODE_INDEX + 1;
  list->end_index = NEW_NODE_INDEX;
  list->length++;
  return list;
}

size_t sl_list_alloc_extent(const sl_list *const list) {
  const size_t HEADER_SIZE = sizeof(sl_list);
  const size_t PADDING_BYTES = get_padding_bytes(list);
  const size_t DATA_ALLOC = list->data_allocation;
  return HEADER_SIZE + PADDING_BYTES + DATA_ALLOC;
}

void sl_list_delete(sl_list **const list) {
  sl_list *const actual_list = *list;
  delete_stack(&actual_list->deleted_nodes);
  free(actual_list);
  *list = NULL;
}

void sl_list_delete_s(sl_list **const list) {
  sl_list *const actual_list = *list;
  const size_t LIST_ALLOC = sl_list_alloc_extent(actual_list);

  memset(actual_list, 0, LIST_ALLOC);
  delete_stack_s(&actual_list->deleted_nodes);
  free(actual_list);
  *list = NULL;
}

static void *print_node(sl_list *list, sl_node *node) {
  printf("%d, ", *(int *)sl_list_get_node_value(list, node));
  return NULL;
}

/* using naive method (for now) */
static size_t fill_unallocated_gaps(sl_list *const list) {
  const size_t OVERALL_LENGTH = sl_list_overall_length(list);
  const size_t VALUE_SIZE = list->value_size;
  byte *const VALUES_END = get_values(list) + OVERALL_LENGTH * VALUE_SIZE;
  sl_node *const NODES_END = get_nodes(list) + OVERALL_LENGTH;

  sl_node *deleted_node = get_deleted_node(list);
  while (deleted_node != NULL) {
    {
      const size_t NODES_RANGE = (byte *)NODES_END - (byte *)deleted_node;
      memmove(deleted_node, deleted_node + 1, NODES_RANGE);
      exit(0);
    }
    {
      byte *const deleted_value = sl_list_get_node_value(list, deleted_node);
      const size_t VALUES_RANGE = (byte *)VALUES_END - deleted_value;
      memmove(deleted_value, deleted_value + VALUE_SIZE, VALUES_RANGE);
    }
    deleted_node = get_deleted_node(list);
  }

  const size_t IDEAL_ALLOC_SIZE = list->length * node_value_pair_size(list);
  return IDEAL_ALLOC_SIZE;
}

sl_list *sl_list_shrink_to_fit(sl_list *const list) {
  const size_t new_data_alloc = fill_unallocated_gaps(list);
  printf("shrink_to_fit(): %zu %zu\n", new_data_alloc, list->data_allocation);
  return sl_list_resize(list, new_data_alloc);
}

int main(void) {
  static const int data[] = {38,    7719, 21238, 2437,  8855,
                             11797, 8365, 32285, 10450, 30612};

  sl_list *list = new_sl_list(data);
  printf("data_alloc: %zu\ntotal allocation: %zu\n", list->data_allocation,
         sl_list_alloc_extent(list));

  sl_list_traverse(list, print_node, NULL);

  sl_list_delete_node(list, get_node_from_index(list, list->start_index));
  putchar('\n');
  sl_list_traverse(list, print_node, NULL);
  list = sl_list_shrink_to_fit(list);

  printf("\ndata_alloc: %zu\ntotal allocation: %zu\nunused nodes: %zu\n",
         list->data_allocation, sl_list_alloc_extent(list),
         sl_list_unused_length(list));

  return 0;
}
