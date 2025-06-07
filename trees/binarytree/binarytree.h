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

typedef void (*bt_op)(binary_tree(void) *, bt_node, void *args);

typedef enum {
  IS_LEFT = 0,
  IS_RIGHT = IS_LEFT + 1,
  NO_LINK = IS_RIGHT + 1
} bt_linkage;

#define NULL_INDEX ((size_t)-1)

/* - TREE MANIPULATION - */

#define bt_node_add(tree, parent, add_to_left) \
  bt_untyped_node_add((void **)&(tree), parent, add_to_left, sizeof *(tree))

/* - NODE MACROS - */

#define bt_has_left(node) ((node)->left != NULL_INDEX)

#define bt_has_parent(node) ((node)->parent != NULL_INDEX)

#define bt_has_right(node) ((node)->right != NULL_INDEX)

#define bt_left(tree, node) bt_untyped_left(tree, node, sizeof *(tree))

#define bt_node_index(tree, node) ((node) - bt_node_arena(tree))

#define bt_parent(tree, node) bt_untyped_parent(tree, node, sizeof *(tree))

#define bt_right(tree, node) bt_untyped_right(tree, node, sizeof *(tree))

#define bt_value(tree, node) ((tree)[bt_node_index(tree, node)])

/* - TREE MACROS - */

#define const_bt_header(tree) ((const_bt_header)(tree) - 1)

#define bt_get_node(tree, index) (bt_node_arena(tree) + (index))

#define bt_has_root(tree) (const_bt_header(tree)->root != NULL_INDEX)

#define bt_header(tree) ((bt_header)(tree) - 1)

#define bt_node_arena(tree) \
  ((bt_node)((byte *)((tree) + bt_total_nodes(tree)) + bt_padding(tree)))

#define bt_non_header_size(tree) \
  (const_bt_header(tree)->allocation - sizeof(struct bt_header))

#define bt_nv_pair_size(tree) (sizeof(struct bt_node) + sizeof *(tree))

#define bt_padding(tree) \
  (bt_non_header_size(tree) - (bt_total_nodes(tree) * bt_nv_pair_size(tree)))

#define bt_root(tree) (bt_get_node(tree, bt_header(tree)->root))

#define bt_root_s(tree) bt_untyped_root(tree, sizeof *(tree))

#define bt_total_nodes(tree) (bt_non_header_size(tree) / bt_nv_pair_size(tree))

#define bt_traverse(tree, op, args) \
  bt_untyped_traverse((void **)&(tree), op, sizeof *(tree), args)

/* - FUNCTIONS - */

bt_node bt_untyped_node_add(binary_tree(void) *, bt_node parent,
                            bool add_to_left, size_t value_size);

void bt_untyped_delete(binary_tree(void) *);

void *bt_untyped_get_value(binary_tree(void), const_bt_node node,
                           size_t value_size);

bt_node bt_untyped_left(binary_tree(void), const_bt_node, size_t value_size);

bt_linkage bt_untyped_link_type(void *tree, const_bt_node child,
                                const_bt_node parent, size_t value_size);

void bt_untyped_node_delete(binary_tree(void), bt_node, size_t value_size);

binary_tree(void) bt_untyped_new(size_t capacity, size_t value_size);

bt_node bt_untyped_parent(binary_tree(void), const_bt_node, size_t value_size);

bt_node bt_untyped_right(binary_tree(void), const_bt_node, size_t value_size);

bt_node bt_untyped_root(binary_tree(void), size_t value_size);

void *bt_untyped_set_value(binary_tree(void), const_bt_node, const void *value,
                           size_t value_size);

void bt_untyped_traverse(binary_tree(void) *, bt_op, size_t value_size,
                         void *args);

#endif
