#include "singlylinkedlist.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte_t;

static s_linked_list *s_list_alloc(const size_t alloc_size) {
  return malloc(alloc_size + sizeof(s_linked_list));
}

s_linked_list *_new_s_list(const void *const data, const size_t num_elems,
                           const size_t elem_size) {
  s_linked_list *const list = new_empty_s_list(num_elems, elem_size);
  if (list == NULL) return NULL;

  /* The nodes are stored past the list header. */
  const size_t NODE_SIZE = elem_size + sizeof(s_linked_node);
  s_linked_node *const nodes_mem = (void *)(list + 1);

  s_linked_node *cur_node = nodes_mem;
  s_linked_node *prev_node = NULL;
  /* The first element of `data` will always be the first node in the list. */
  list->start_node = nodes_mem;
  for (size_t i = 0; i < num_elems; i++) {
    const void *const cur_elem = (char *)data + i * elem_size;
    /* The value stored at the node should be ahead of the node header. */
    cur_node->value = nodes_mem + 1;
    memcpy(cur_node->value, cur_elem, elem_size);
    nodes_mem->next = NULL;
    if (prev_node != NULL) prev_node->next = cur_node;
    cur_node = (void *)((byte_t *)cur_node + NODE_SIZE);
  }
  /* The last element of `data` will always be the last node in the list. */
  list->end_node = cur_node;

  return list;
}

s_linked_list *new_empty_s_list(const size_t num_elems,
                                const size_t elem_size) {
  const size_t ALLOC_SIZE = elem_size * num_elems;
  s_linked_list *list = s_list_alloc(ALLOC_SIZE);
  if (list == NULL) return NULL;
  list->capacity = ALLOC_SIZE;
  list->used_capacity = ALLOC_SIZE;
  list->start_node = NULL;
  list->end_node = NULL;

  return list;
}

int main(void) {
  const int data[] = {1, 2, 3, 4, 5, 6};
  s_linked_list *list = new_s_list(data);
  return 0;
}
