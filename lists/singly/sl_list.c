#include "sl_list.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

static inline size_t node_value_pair_size(const sl_list *const list) {
  const size_t VALUE_SIZE = list->value_size;
  return sizeof(sll_node) + VALUE_SIZE;
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
  const size_t SIZE_DIFF = values_alloc % alignof(sll_node);
  if (SIZE_DIFF == 0) return 0;
  const size_t PADDING_BYTES = alignof(sll_node) - SIZE_DIFF;
  return PADDING_BYTES;
}

static inline size_t get_padding_bytes(const sl_list *const list) {
  const size_t VALUES_ALLOC = get_values_alloc(list);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  return PADDING_BYTES;
}

static inline sll_node *get_nodes(sl_list *const list) {
  const size_t VALUES_ALLOC = get_values_alloc(list);
  const size_t PADDING_BYTES = get_padding_bytes(list);
  byte *const VALUES = get_values(list);
  byte *const VALUES_END = VALUES + VALUES_ALLOC;
  sll_node *const NODES = (void *)(VALUES_END + PADDING_BYTES);
  return NODES;
}

void *get_node_value(sl_list *const list, const sll_node *const node) {
  const size_t VALUE_SIZE = list->value_size;
  const size_t NODE_VALUE_OFFSET = VALUE_SIZE * node->value_index;
  byte *const VALUES = get_values(list);
  byte *const NODE_VALUE = VALUES + NODE_VALUE_OFFSET;
  return NODE_VALUE;
}

sl_list *init_sl_list(const size_t num_nodes, const size_t value_size) {
  const size_t VALUES_ALLOC = num_nodes * value_size;
  const size_t NODES_ALLOC = num_nodes * sizeof(sll_node);
  const size_t PADDING_BYTES = calc_padding_bytes(VALUES_ALLOC);
  const size_t HEADER_ALLOC = sizeof(sl_list);
  const size_t TOTAL_ALLOC =
      HEADER_ALLOC + VALUES_ALLOC + NODES_ALLOC + PADDING_BYTES;

  sl_list *const list = malloc(TOTAL_ALLOC);
  if (list == NULL) return NULL;

  list->start = NULL;
  list->length = 0;
  list->data_allocation = VALUES_ALLOC + NODES_ALLOC;
  list->value_size = value_size;
  return list;
}

sl_list *_new_sl_list(const void *const data, const size_t num_elems,
                      const size_t elem_size) {
  sl_list *const list = init_sl_list(num_elems, elem_size);
  if (list == NULL) return NULL;

  byte *const VALUES_MEM = get_values(list);
  sll_node *const NODES_MEM = get_nodes(list);

  sll_node *cur_node = NODES_MEM;
  sll_node *prev_node = NULL;
  for (size_t i = 0; i < num_elems; i++) {
    const void *const cur_elem = (byte *)data + i * elem_size;
    void *const cur_value = VALUES_MEM + i * elem_size;
    memcpy(cur_value, cur_elem, elem_size);
    cur_node->value_index = i;
    cur_node->next_node_index = 0;
    if (prev_node != NULL) prev_node->next_node_index = i - 1;
    /*
     * Incrementing to the next node since `cur_node` is a copy of the pointer
     * to `NODES_MEM`.
     */
    cur_node++;
  }
  list->start = NODES_MEM;

  return list;
}

int main(void) {
  const int data[] = {1, 2, 3, 4, 5};
  sl_list *list = new_sl_list(data);
  printf("%d", *(int*)get_node_value(list, list->start + 1));
  return 0;
}
