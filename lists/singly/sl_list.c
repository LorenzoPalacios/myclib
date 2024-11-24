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

static inline size_t sll_overall_length(const sl_list *const list) {
  const size_t ALLOC = list->data_allocation;
  const size_t PAIR_SIZE = node_value_pair_size(list);
  return ALLOC / PAIR_SIZE;
}

static inline size_t sll_unused_length(const sl_list *const list) {
  const size_t OVERALL_LENGTH = sll_overall_length(list);
  const size_t USED_LENGTH = list->length;
  const size_t UNUSED_LENGTH = OVERALL_LENGTH - USED_LENGTH;
  return UNUSED_LENGTH;
}

static inline size_t get_values_alloc(const sl_list *const list) {
  const size_t LENGTH = sll_overall_length(list);
  const size_t VALUE_SIZE = list->value_size;
  const size_t VALUES_ALLOC = LENGTH * VALUE_SIZE;
  return VALUES_ALLOC;
}

static inline void *get_values(sl_list *const list) { return list + 1; }

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

void *get_node_value(sl_list *const list, const sl_node *const node) {
  const size_t VALUE_SIZE = list->value_size;
  const size_t NODE_VALUE_OFFSET = VALUE_SIZE * node->value_index;
  byte *const VALUES = get_values(list);
  byte *const NODE_VALUE = VALUES + NODE_VALUE_OFFSET;
  return NODE_VALUE;
}

sl_list *init_sl_list(const size_t num_nodes, const size_t value_size) {
  const size_t VALUES_ALLOC = num_nodes * value_size;
  const size_t NODES_ALLOC = num_nodes * sizeof(sl_node);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  const size_t HEADER_ALLOC = sizeof(sl_list);
  const size_t TOTAL_ALLOC =
      HEADER_ALLOC + VALUES_ALLOC + NODES_ALLOC + PADDING_BYTES;

  sl_list *const list = malloc(TOTAL_ALLOC);
  if (list == NULL) return NULL;

  list->start = list->end = NULL;
  list->length = 0;
  list->data_allocation = VALUES_ALLOC + NODES_ALLOC;
  list->value_size = value_size;
  return list;
}

static inline sl_node *get_next_node(sl_list *const list, sl_node *const node) {
  if (node->next_node_index >= list->length) return NULL;
  sl_node *const NODES = get_nodes(list);
  sl_node *const NEXT_NODE = NODES + node->next_node_index;
  return NEXT_NODE;
}

void *sl_list_traverse(sl_list *list,
                       void *(*const op)(sl_list *list, sl_node *cur_node),
                       bool (*const condition)(sl_list *list,
                                               sl_node *cur_node)) {
  void *latest_op_value = NULL;
  sl_node *cur_node = list->start;
  while (cur_node != NULL) {
    if (op != NULL) latest_op_value = op(list, cur_node);
    if (condition != NULL)
      if (!condition(list, cur_node)) break;
    cur_node = get_next_node(list, cur_node);
  }
  return latest_op_value;
}

sl_list *_new_sl_list(const void *const data, const size_t num_elems,
                      const size_t elem_size) {
  sl_list *const list = init_sl_list(num_elems, elem_size);
  if (list == NULL) return NULL;

  byte *const VALUES_MEM = get_values(list);
  sl_node *const NODES_MEM = get_nodes(list);

  sl_node *cur_node = NODES_MEM;
  for (size_t i = 0; i < num_elems; i++) {
    const void *const cur_elem = (byte *)data + i * elem_size;
    void *const cur_value = VALUES_MEM + i * elem_size;
    memcpy(cur_value, cur_elem, elem_size);
    cur_node->value_index = i;
    cur_node->next_node_index = i + 1;
    /*
     * Incrementing to the next node since `cur_node` is a copy of the pointer
     * to `NODES_MEM`.
     */
    cur_node++;
  }
  list->start = NODES_MEM;
  list->end = cur_node - 1;
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

  /*
   * `data_allocation` should only track the amount of memory used by nodes and
   * values.
   */
  list->data_allocation = DATA_ALLOC;

  sl_node *const NEW_NODES_REGION = get_nodes(list);
  const size_t LENGTH = list->length;
  const size_t SHIFT_OFFSET = LENGTH * sizeof(sl_node);
  memmove(OLD_NODES_REGION, NEW_NODES_REGION, SHIFT_OFFSET);

  list->start = NEW_NODES_REGION;
  list->end = list->start + LENGTH;
  return list;
}

static void *print_index(sl_list *list, sl_node *cur_node) {
  printf("(%d) index: %zu\n", *(int *)get_node_value(list, cur_node),
         cur_node->next_node_index);
  return NULL;
}

int main(void) {
  const int data[] = {1, 2};
  sl_list *list = new_sl_list(data);
  sl_list_traverse(list, print_index, NULL);
  putchar('\n');
  list = sl_list_resize(list, list->data_allocation + 20);
  printf("first node value: %d\n", *(int*)get_node_value(list, get_nodes(list)));
  sl_list_traverse(list, print_index, NULL);
  return 0;
}
