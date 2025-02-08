#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#include <stdbool.h>
#include <stddef.h>

// singly-linked list node
typedef struct sl_node sl_node;

// singly-linked list
typedef struct sl_list sl_list;

#define sll_new(data) \
  sll_new_(data, sizeof(data) / sizeof *(data), sizeof *(data))

#define sll_traverse_list(list, op) sll_traverse(list, sll_head(list), op)

/**
 * @brief Appends a node containing `value` to `list`.
 *
 * @return A pointer to a singly linked list associated with the contents of
 * `list` or `NULL` upon failure.
 */
sl_list *sll_add_node(sl_list *list, const void *value);

size_t sll_capacity(const sl_list *list);

void sll_delete(sl_list *list);

bool sll_delete_node(sl_list *list, sl_node *node);

sl_list *sll_expand(sl_list *list);

sl_node *sll_head(sl_list *list);

sl_list *sll_init(size_t num_nodes, size_t value_size);

/**
 * Creates a new singly linked list whose nodes contain values copied from the
 * contents at `data`.
 *
 * \return A singly linked list whose nodes correspond to elements at `data` or
 * `NULL` upon failure.
 */
sl_list *sll_new_(const void *data, size_t num_values, size_t value_size);

void *sll_node_value(sl_list *list, sl_node *node);

sl_list *sll_resize(sl_list *list, size_t new_capacity);

sl_node *sll_tail(sl_list *list);

sl_list *sll_shrink(sl_list *list);

void sll_swap(sl_list *list, sl_node *node_1, sl_node *node_2);

void sll_traverse(sl_list *list, sl_node *from,
                  bool (*operation)(sl_list *list, sl_node *node));

#endif
