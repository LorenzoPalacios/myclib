#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#include <stddef.h>

#include "../../stack/stack.h"

/* singly-linked node (sl node) */
typedef struct sl_node {
  size_t value_index;
  size_t next_node_index;
} sl_node;

/* singly-linked list (sl list) */
typedef struct sl_list {
  size_t start_index;
  size_t end_index;
  size_t length;
  size_t value_size;
  /*
   * `data_allocation` only tracks the amount of memory used by nodes and
   * values.
   */
  size_t data_allocation;
  stack *deleted_nodes;
} sl_list;

#define new_sl_list(data) \
  _sl_list_create(data, sizeof(data) / sizeof *(data), sizeof *(data))

/*
 * Creates a new singly linked list whose nodes contain values copied from the
 * contents at `data`.
 *
 * \return A singly linked list whose nodes correspond to elements at `data` or
 * `NULL` upon failure.
 */
sl_list *_sl_list_create(const void *data, size_t num_elems, size_t elem_size);

/*
 * Appends a new node containing `val` to `list`, expanding `list` if necessary.
 *
 * \return A pointer associated with the contents of `list` or `NULL` upon
 * failure.
 */
sl_list *sll_add_value(sl_list *list, void *val);

/*
 * Appends `node` to `list`.
 *
 * \return A pointer to a singly linked list associated with the contents of
 * `list` or `NULL` upon failure.
 */
sl_list *sll_add_node(sl_list list, sl_node *node);

#endif
