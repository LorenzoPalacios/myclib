#include "singlylinkedlist.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>

typedef unsigned char byte;

static inline size_t node_value_pair_size(const sl_list *const list) {
  const size_t VALUE_SIZE = list->value_size;
  return sizeof(sll_node) + VALUE_SIZE;
}

static inline size_t list_length_overall(const sl_list *const list) {
  const size_t ALLOC = list->data_allocation;
  const size_t PAIR_SIZE = node_value_pair_size(list);
  return ALLOC / PAIR_SIZE;
}

static inline size_t list_length_used(const sl_list *const list) {
  return list->end - list->start;
}

static inline size_t list_length_unused(const sl_list *const list) {
  const size_t OVERALL_LENGTH = list_length_overall(list);
  const size_t UNUSED_LENGTH = OVERALL_LENGTH - list->length;
  return UNUSED_LENGTH;
}

static inline void *get_values(sl_list *const list) { return list + 1; }

static inline sll_node *get_nodes(sl_list *const list) {
  byte *const VALUES = get_values(list);
  const size_t VALUE_SIZE = list->value_size;
  const size_t LIST_LENGTH = list_length_overall(list);
  return (void *)(VALUES + LIST_LENGTH * VALUE_SIZE);
}

static inline size_t calc_padding_bytes(const size_t values_alloc) {
  const size_t SIZE_DIFF = values_alloc % alignof(sll_node);
  if (SIZE_DIFF == 0) return 0;
  const size_t PADDING_BYTES = alignof(sll_node) - SIZE_DIFF;
  return PADDING_BYTES;
}

sl_list *init_sl_list(const size_t num_nodes, const size_t value_size) {

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
  /*
   * The last element of `data` will always be the last node in the list, which
   * will be `cur_node` upon the above loop's completion.
   */
  list->end = cur_node;

  return list;
}

int main(void) {
  const int data[] = {1, 2, 3, 4, 5, 6};
  sl_list *list = new_sl_list(data);
  return 0;
}
