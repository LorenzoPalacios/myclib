#include "binarytree.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/myclib.h"
#include "../../stack/stack.h"

#define bt_calc_padding_bytes(capacity, value_size) \
  (sizeof(size_t) - ((value_size) * (capacity) % sizeof(size_t)))

#define bt_header_from_ref(ref) (((bt_header) * (void **)(ref)) - 1)

/* - INTERNAL FUNCTION DECLARATIONS - */

static size_t bt_untyped_nv_pair_size(size_t value_size);

static bt_node bt_untyped_node_arena(void *tree, size_t value_size);

static size_t bt_untyped_total_nodes(const void *tree, size_t value_size);

/* - INTERNAL FUNCTION DEFINITIONS - */

static inline size_t bt_cur_padding_bytes(const void *const tree, const size_t value_size) {
  return bt_non_header_size(tree) - (bt_untyped_nv_pair_size(value_size) *
                                     bt_untyped_total_nodes(tree, value_size));
}

static inline bt_node bt_untyped_get_node(void *const tree, const size_t index,
                                          const size_t value_size) {
  return bt_untyped_node_arena(tree, value_size) + index;
}

static inline size_t bt_untyped_nv_pair_size(const size_t value_size) {
  return sizeof(struct bt_node) + value_size;
}

static inline bt_node bt_untyped_node_arena(void *const tree,
                                            const size_t value_size) {
  return (bt_node)((byte *)tree + bt_cur_padding_bytes(tree, value_size) +
                   (bt_untyped_total_nodes(tree, value_size) * value_size));
}

static inline size_t bt_untyped_total_nodes(const void *const tree,
                                            const size_t value_size) {
  return bt_non_header_size(tree) / bt_untyped_nv_pair_size(value_size);
}

static inline size_t bt_untyped_node_index(void *const tree, const_bt_node node,
                                           const size_t value_size) {
  return (size_t)(node -
                  (const_bt_node)bt_untyped_node_arena(tree, value_size));
}

static inline bt_node bt_untyped_get_deleted_node(void *const tree,
                                                  const size_t value_size) {
  stack(size_t) deleted_nodes = bt_header(tree)->deleted_nodes;
  if (!stack_is_empty(deleted_nodes))
    return bt_untyped_get_node(tree, stack_pop(deleted_nodes), value_size);
  return NULL;
}

static inline bt_node bt_untyped_get_unused_node(void *const tree,
                                                 const size_t value_size) {
  return bt_untyped_get_node(tree, bt_header(tree)->active_nodes, value_size);
}

static inline bt_node bt_untyped_get_open_node(void **const tree,
                                               const size_t value_size) {
  bt_node candidate = bt_untyped_get_deleted_node(*tree, value_size);
  if (candidate == NULL)
    candidate = bt_untyped_get_unused_node(*tree, value_size);
  return candidate;
}

#define bt_node_get_child_slot(node, should_take_left)                      \
  ((should_take_left) ? ((node)->left == NULL_INDEX ? &(node)->left : NULL) \
                      : ((node)->right == NULL_INDEX ? &(node)->right : NULL))

/* - FUNCTIONS - */

bt_node bt_untyped_add_node(void **const tree, bt_node parent,
                            const bool add_to_left, const void *const value,
                            const size_t value_size) {
  void *const tree_actual = *tree;
  bt_header header = bt_header(tree_actual);
  bt_node new_node = bt_untyped_get_open_node(tree, value_size);
  if (new_node == NULL) return NULL;
  if (header->root == NULL_INDEX) {
    header->root = bt_untyped_node_index(tree_actual, new_node, value_size);
    new_node->parent = NULL_INDEX;
  } else if (parent != NULL) {
    size_t *const child_index = bt_node_get_child_slot(parent, add_to_left);
    if (child_index == NULL) return NULL;
    *child_index = bt_untyped_node_index(tree_actual, new_node, value_size);
    new_node->parent = bt_untyped_node_index(tree_actual, parent, value_size);
  } else {
    return NULL;
  }
  bt_node_initialize(new_node);
  bt_untyped_set_value(tree_actual, new_node, value, value_size);
  header->active_nodes++;
  return new_node;
}

void *bt_untyped_get_value(void *const tree, const_bt_node node,
                           const size_t value_size) {
  return (byte *)tree +
         (value_size * bt_untyped_node_index(tree, node, value_size));
}

void *bt_untyped_new(const size_t capacity, const size_t value_size) {
  const size_t PADDING = bt_calc_padding_bytes(capacity, value_size);
  const size_t ALLOCATION = (bt_untyped_nv_pair_size(value_size) * capacity) +
                            PADDING + sizeof(struct bt_header);
  bt_header tree = malloc(ALLOCATION);
  if (tree == NULL) return NULL;
  tree->deleted_nodes = stack_new(size_t, 3);
  tree->active_nodes = 0;
  tree->allocation = ALLOCATION;
  tree->root = NULL_INDEX;
  return tree + 1;
}

void *bt_untyped_set_value(void *const tree, const_bt_node node,
                           const void *const value, const size_t value_size) {
  void *const dst = bt_untyped_get_value(tree, node, value_size);
  return memcpy(dst, value, value_size);
}
