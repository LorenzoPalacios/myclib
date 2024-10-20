#include "singlylinkedlist.h"

#include <stddef.h>
#include <stdlib.h>

static s_linked_list *s_list_alloc(const size_t alloc_size) {
  return malloc(alloc_size + sizeof(s_linked_list));
}

s_linked_list *_new_s_list(const void *const data, const size_t num_elems,
                           const size_t elem_size) {
  s_linked_list *list = new_empty_s_list(num_elems, elem_size);
  if (list == NULL) return NULL;

  /* The nodes are stored past the list header. */
  const size_t NODE_SIZE = elem_size + sizeof(s_linked_node);
  s_linked_node *nodes_mem = (void *)(list + 1);

  /* The first element of `data` will always be the first node in the list. */
  list->start_node = nodes_mem;
  for (size_t i = 0; i < num_elems; i++) {
    
    nodes_mem = (void*)((char *)nodes_mem + NODE_SIZE);
  }

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
