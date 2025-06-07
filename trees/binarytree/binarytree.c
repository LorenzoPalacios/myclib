#include "binarytree.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/myclib.h"
#include "../../stack/stack.h"

/* - CONVENIENCE MACROS - */

#define bt_calc_padding_init(capacity, value_size) \
  ((value_size) == sizeof(size_t)                  \
       ? 0                                         \
       : sizeof(size_t) - ((value_size) * (capacity) % sizeof(size_t)))

#define bt_header_from_ref(ref) (((bt_header) * (void **)(ref)) - 1)

#define bt_node_get_child_slot(node, should_take_left)                      \
  ((should_take_left) ? ((node)->left == NULL_INDEX ? &(node)->left : NULL) \
                      : ((node)->right == NULL_INDEX ? &(node)->right : NULL))

/* - FUNCTION DECLARATIONS - */

static bt_node bt_untyped_get_node(void *tree, size_t index, size_t value_size);
static bt_node bt_untyped_node_arena(void *tree, size_t value_size);
static size_t bt_untyped_nv_pair_size(size_t value_size);
static size_t bt_untyped_total_nodes(const void *tree, size_t value_size);
static bt_node bt_untyped_get_unused_node(void *tree, size_t value_size);

/* - TREE INTERNAL - */

static inline size_t bt_untyped_padding(const void *const tree,
                                        const size_t value_size) {
  return bt_non_header_size(tree) - (bt_untyped_nv_pair_size(value_size) *
                                     bt_untyped_total_nodes(tree, value_size));
}

static inline bt_node bt_untyped_get_deleted_node(void *const tree,
                                                  const size_t value_size) {
  stack(size_t) deleted_nodes = bt_header(tree)->deleted_nodes;
  if (!stack_is_empty(deleted_nodes))
    return bt_untyped_get_node(tree, stack_pop(deleted_nodes), value_size);
  return NULL;
}

static inline bt_node bt_untyped_get_node(void *const tree, const size_t index,
                                          const size_t value_size) {
  return bt_untyped_node_arena(tree, value_size) + index;
}

static inline bt_node bt_untyped_get_open_node(void **const tree,
                                               const size_t value_size) {
  bt_node candidate = bt_untyped_get_deleted_node(*tree, value_size);
  if (candidate == NULL)
    candidate = bt_untyped_get_unused_node(*tree, value_size);
  return candidate;
}

static inline bt_node bt_untyped_get_unused_node(void *const tree,
                                                 const size_t value_size) {
  return bt_untyped_get_node(tree, bt_header(tree)->active_nodes, value_size);
}

static inline bool bt_untyped_has_root(void *const tree) {
  return bt_header(tree)->root != NULL_INDEX;
}

static inline bt_node bt_untyped_node_arena(void *const tree,
                                            const size_t value_size) {
  return (bt_node)((byte *)tree + bt_untyped_padding(tree, value_size) +
                   (bt_untyped_total_nodes(tree, value_size) * value_size));
}

size_t bt_untyped_node_index(void *const tree, const_bt_node node,
                             const size_t value_size) {
  return (size_t)(node -
                  (const_bt_node)bt_untyped_node_arena(tree, value_size));
}

static inline size_t bt_untyped_nv_pair_size(const size_t value_size) {
  return sizeof(struct bt_node) + value_size;
}

static inline size_t bt_untyped_total_nodes(const void *const tree,
                                            const size_t value_size) {
  return bt_non_header_size(tree) / bt_untyped_nv_pair_size(value_size);
}

/* - NODE INTERNAL - */

static inline bool bt_untyped_node_is_left(void *const tree,
                                           const_bt_node child,
                                           const_bt_node parent,
                                           const size_t value_size) {
  return parent->left == bt_untyped_node_index(tree, child, value_size);
}

static inline bt_node bt_untyped_node_parent(void *const tree,
                                             const_bt_node node,
                                             const size_t value_size) {
  return bt_untyped_node_arena(tree, value_size) + node->parent;
}

static inline void bt_untyped_node_disconnect(void *const tree, bt_node node,
                                              const size_t value_size) {
  if (bt_has_parent(node)) {
    bt_node parent = bt_untyped_node_parent(tree, node, value_size);
    if (bt_untyped_node_is_left(tree, node, parent, value_size))
      parent->left = NULL_INDEX;
    else
      parent->right = NULL_INDEX;
    node->parent = NULL_INDEX;
  }
}

static inline void bt_node_initialize(bt_node node) {
  node->left = node->right = NULL_INDEX;
}

/* - FUNCTIONS - */

bt_node bt_untyped_node_add(void **const tree, bt_node parent,
                            const bool add_to_left, const size_t value_size) {
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
  header->active_nodes++;
  return new_node;
}

inline void bt_untyped_delete(void **const tree) {
  bt_header header = bt_header_from_ref(tree);
  stack_delete(header->deleted_nodes);
  free(header);
  *tree = NULL;
}

void bt_untyped_node_delete(void *const tree, bt_node node,
                            const size_t value_size) {
  stack_push(bt_header(tree)->deleted_nodes,
             bt_untyped_node_index(tree, node, value_size));
  bt_untyped_node_disconnect(tree, node, value_size);
}

void *bt_untyped_get_value(void *const tree, const_bt_node node,
                           const size_t value_size) {
  return (byte *)tree +
         (value_size * bt_untyped_node_index(tree, node, value_size));
}

bt_node bt_untyped_left(void *const tree, const_bt_node node,
                        const size_t value_size) {
  return bt_untyped_node_arena(tree, value_size) + node->left;
}

bt_linkage bt_untyped_link_type(void *const tree, const_bt_node child,
                                const_bt_node parent, const size_t value_size) {
  const size_t CHILD_INDEX = bt_untyped_node_index(tree, child, value_size);
  if (CHILD_INDEX == parent->left) return IS_LEFT;
  if (CHILD_INDEX == parent->right) return IS_RIGHT;
  return NO_LINK;
}

void *bt_untyped_new(const size_t capacity, const size_t value_size) {
  const size_t PADDING = bt_calc_padding_init(capacity, value_size);
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

bt_node bt_untyped_parent(void *const tree, const_bt_node node,
                          const size_t value_size) {
  return bt_untyped_node_arena(tree, value_size) + node->parent;
}

bt_node bt_untyped_right(void *const tree, const_bt_node node,
                         const size_t value_size) {
  return bt_untyped_node_arena(tree, value_size) + node->right;
}

bt_node bt_untyped_root(void *const tree, const size_t value_size) {
  return bt_untyped_has_root(tree)
             ? bt_untyped_get_node(tree, bt_header(tree)->root, value_size)
             : NULL;
}

void *bt_untyped_set_value(void *const tree, const_bt_node node,
                           const void *const value, const size_t value_size) {
  void *const dst = bt_untyped_get_value(tree, node, value_size);
  return memcpy(dst, value, value_size);
}

void bt_untyped_traverse(void **const tree_ref, bt_op op,
                         const size_t value_size, void *args) {
  void *tree = *tree_ref;
  stack(size_t) branches = stack_new(size_t, bt_header(tree)->active_nodes / 3);

  bt_node cur_node = bt_untyped_root(tree, value_size);
  while (cur_node != NULL) {
    op(tree_ref, cur_node, args);
    if (bt_has_left(cur_node)) {
      if (bt_has_right(cur_node)) {
        const size_t CUR_NODE_INDEX =
            bt_untyped_node_index(tree, cur_node, value_size);
        stack_push_s(branches, CUR_NODE_INDEX);
      }
      cur_node = bt_untyped_left(tree, cur_node, value_size);
    } else if (bt_has_right(cur_node)) {
      cur_node = bt_untyped_right(tree, cur_node, value_size);
    } else {
      if (!stack_is_empty(branches)) {
        const_bt_node prev =
            bt_untyped_get_node(tree, stack_pop(branches), value_size);
        cur_node = bt_untyped_right(tree, prev, value_size);
      } else {
        break;
      }
    }
  }
  stack_delete(branches);
}
