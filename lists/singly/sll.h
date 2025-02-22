#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L)
#if (__STDC_VERSION__ < 202311L)
/* For C99 to C17. */
#include <stdbool.h>
#endif
#else
/* For C95 and below. */
#if (!(defined true || defined false))
typedef unsigned char bool;
#define true (1)
#define false (0)
#endif
#define inline
#endif

#include <stddef.h>

/* singly-linked list node */
typedef struct sl_node sl_node;

/* singly-linked list */
typedef struct sl_list sl_list;

#define sll_add_node(list, value) sll_add_node_(&(list), value)

#define sll_delete(list) sll_delete_(&(list))

#define sll_delete_node(list, node) sll_delete_node_(&(list), node)

#define sll_expand(list) sll_expand_(&(list))

#define sll_head(list) sll_head_(&(list))

#define sll_init(type, num_nodes) sll_init_(sizeof(type), num_nodes)

#define sll_new(data) \
  sll_new_(data, sizeof(data) / sizeof *(data), sizeof *(data))

#define sll_next_node(list, node) sll_next_node_(&(list), node)

#define sll_node_value(list, node) sll_node_value_(&(list), node)

#define sll_prev_node(list, node) sll_prev_node_(&(list), node)

#define sll_resize(list, new_capacity) sll_resize_(&(list), new_capacity)

#define sll_shrink(list) sll_shrink_(&(list))

#define sll_swap(list, node_1, node_2) sll_swap(&(list), node_1, node_2)

#define sll_tail(list) sll_tail_(&(list))

#define sll_traverse_list(list, op) sll_traverse(list, sll_head(list), op)

/**
 * @brief Appends a node containing `value` to `list`.
 *
 * @return `true` if a node associated with the value was successfully added to
 * the list. `false` otherwise.
 */
bool sll_add_node_(sl_list *list, const void *value);

/**
 * @brief Deletes a list, invalidating all accesses to its contents.
 *
 * @note The list object itself may remain usable, however it is guaranteed that
 * the nodes composing the list are not.
 */
void sll_delete_(const sl_list *list);

bool sll_delete_node_(sl_list *list, const sl_node *node);

bool sll_expand_(sl_list *list);

sl_node *sll_head_(const sl_list *list);

sl_list sll_init_(size_t value_size, size_t num_nodes);

/**
 * Creates a new singly linked list whose nodes contain values copied from the
 * contents at `data`.
 *
 * \return A singly linked list whose nodes correspond to elements at `data` or
 * `NULL` upon failure.
 */
sl_list sll_new_(const void *data, size_t num_values, size_t value_size);

sl_node *sll_next_node_(const sl_list *list, const sl_node *node);

void *sll_node_value_(const sl_list *list, const sl_node *node);

sl_node *sll_prev_node_(const sl_list *list, const sl_node *node);

bool sll_resize_(sl_list *list, size_t new_capacity);

sl_node *sll_tail_(const sl_list *list);

bool sll_shrink_(sl_list *list);

void sll_swap_(const sl_list *list, sl_node *node_1, sl_node *node_2);

void sll_traverse_(sl_list *list, sl_node *from,
                   bool (*operation)(sl_list *list, sl_node *node));

#endif
