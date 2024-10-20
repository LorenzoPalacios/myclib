#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#include <stddef.h>

typedef struct singly_linked_node {
  void *data;
  struct singly_linked_node *next;
} s_linked_node;

typedef struct singly_linked_list {
  s_linked_node *start_node;
  s_linked_node *end_node;
  size_t capacity;
  size_t used_capacity;
} s_linked_list;

#define new_s_list(data) \
  _new_s_list(data, sizeof(data) / sizeof *(data), sizeof *(data))

/*
 * Creates a new singly linked list whose nodes contain values copied from the
 * contents at `data`.
 *
 * \return A singly linked list whose nodes correspond to elements at `data` or
 * `NULL` upon failure.
 */
s_linked_list *_new_s_list(const void *data, size_t num_elems,
                           size_t elem_size);

/*
 * Creates a new singly linked list with no nodes.
 *
 * \return An empty singly linked list or `NULL` upon failure.
 */
s_linked_list *new_empty_s_list(size_t num_elems, size_t elem_size);

#endif
