#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stddef.h>

#include "../../include/myclib.h"
#include "../../stack/stack.h"

/* - DEFINITIONS - */

#define binary_tree(type) type *

#define binary_tree_new(type, capacity) \
  ((type *)bt_untyped_new(capacity, sizeof(type)))

struct bt_node {
  size_t left;
  size_t right;
  size_t parent;
};

struct bt_header {
  stack(size_t) deleted_nodes;
  size_t active_nodes;
  size_t allocation;
  size_t root;
};

typedef struct bt_node *bt_node;

typedef struct bt_header *bt_header;

typedef const struct bt_node *const_bt_node;

typedef const struct bt_header *const_bt_header;

#define NULL_INDEX ((size_t)-1)

/* - TREE MANIPULATION - */

#define bt_add_node_s(tree, parent, add_to_left, value)                \
  bt_untyped_add_node((void **)&(tree), parent, add_to_left, &(value), \
                      sizeof *(tree))

#define bt_get_deleted_node(tree)                                     \
  (!stack_is_empty(bt_header(tree))                                   \
       ? bt_get_node(tree, stack_pop(bt_header(tree)->deleted_nodes)) \
       : NULL)

#define bt_get_node(tree, index) (bt_node_arena(tree) + (index))

#define bt_get_open_node(tree)                                        \
  (!stack_is_empty((tree)->deleted_nodes) ? bt_get_deleted_node(tree) \
                                          : bt_get_unused_node(tree))

#define bt_get_unused_node(tree)                          \
  ((bt_header(tree))->active_nodes < bt_total_nodes(tree) \
       ? bt_get_node(tree, bt_header(tree)->active_nodes) \
       : NULL)

#define bt_has_root(tree) (bt_header(tree)->root != NULL_INDEX)

#define bt_header(tree) ((bt_header)(tree) - 1)

#define bt_header_const(tree) ((const struct bt_header *)(tree) - 1)

#define bt_padding(tree) \
  (sizeof(size_t) - ((sizeof *(tree) * bt_total_nodes(tree)) % sizeof(size_t)))

#define bt_root_node(tree) \
  (bt_has_root(tree) ? bt_get_node(tree, bt_header(tree)->root) : NULL)

#define bt_total_nodes(tree) (bt_non_header_size(tree) / bt_nv_pair_size(tree))

#define bt_node_arena(tree) \
  ((bt_node)((byte *)((tree) + bt_total_nodes(tree)) + bt_padding(tree)))

#define bt_non_header_size(tree) \
  (bt_header_const(tree)->allocation - sizeof(struct bt_header))

#define bt_nv_pair_size(tree) (sizeof(struct bt_node) + sizeof *(tree))

/* - NODE MANIPULATION - */

#define bt_node_delete(tree, node)                                       \
  stack_push(bt_header(tree)->deleted_nodes, bt_node_index(tree, node)); \
  bt_node_disconnect(tree, node)

#define bt_node_disconnect(tree, node)                           \
  if (bt_node_has_parent(node)) {                                \
    if (bt_node_is_left(tree, node, bt_node_parent(tree, node))) \
      bt_node_parent(tree, node)->left = NULL_INDEX;             \
    else                                                         \
      bt_node_parent(tree, node)->right = NULL_INDEX;            \
  }                                                              \
  (void)0

#define bt_node_get_value(tree, node) ((tree)[bt_node_index(tree, node)])

#define bt_node_has_left(node) ((node)->left != NULL_INDEX)

#define bt_node_has_parent(node) ((node)->parent != NULL_INDEX)

#define bt_node_has_right(node) ((node)->right != NULL_INDEX)

#define bt_node_index(tree, node) ((size_t)((node) - bt_node_arena(tree)))

#define bt_node_initialize(node) \
  (void)((node)->left = (node)->right = NULL_INDEX)

#define bt_node_is_root(tree, node) \
  (bt_node_arena(tree) + bt_header(tree)->root == (node))

#define bt_node_is_left(tree, child, parent) \
  ((parent)->left == bt_node_index(tree, child))

#define bt_node_is_right(tree, child, parent) \
  ((parent)->right == bt_node_index(tree, child))

#define bt_node_left(tree, node) \
  (bt_node_has_left(node) ? bt_get_node(tree, (node)->left) : NULL)

#define bt_node_right(tree, node) \
  (bt_node_has_right(node) ? bt_get_node(tree, (node)->right) : NULL)

#define bt_node_parent(tree, node) \
  (bt_node_has_parent(node) ? bt_get_node(tree, (node)->right) : NULL)

#define bt_node_value(tree, node) ((tree)[bt_node_index(tree, node)])

/* - FUNCTIONS - */

bt_node bt_untyped_add_node(binary_tree(void) *, bt_node parent,
                            bool add_to_left, const void *value,
                            size_t value_size);

void *bt_untyped_get_value(binary_tree(void), const_bt_node node,
                           size_t value_size);

void *bt_untyped_set_value(binary_tree(void), const_bt_node, const void *value,
                           size_t value_size);

binary_tree(void) bt_untyped_new(size_t capacity, size_t value_size);

void bt_untyped_traverse(binary_tree(void), size_t value_size);

#endif
