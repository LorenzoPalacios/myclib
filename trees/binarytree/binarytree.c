#include "binarytree.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

typedef struct bt_node {
  size_t parent_index;
  size_t left_index;
  size_t right_index;
} bt_node;

/**
 * `deleted_nodes` is a `stack` containing indicies to deleted nodes
 * (relative to `get_nodes()`). `deleted_nodes` will never contain `NULL_INDEX`,
 * but will return a `NULL` pointer to an index when empty.
 *
 * `allocation` describes the total amount of bytes allocated for the tree.
 *
 * `root_index` describes the index of the root node
 * (relative to `get_nodes()`).
 *
 * `value_size` describes the size of a value.
 *
 * `unallocated_region_index` is the current index of the unallocated region
 * within the nodes segment (distinct from deleted nodes).
 */
typedef struct binary_tree {
  stack *deleted_nodes;
  size_t allocation;  // Total bytes allocated for the tree and nodes.
  size_t root_index;
  size_t value_size;  // Size (in bytes) of a stored value.
  size_t unallocated_region_index;
} binary_tree;

#include "../../stack/stack.h"

// Reallocation factor used by `bt_expand()`.
#define EXPANSION_FACTOR (2)

// - GETTERS BEGIN -

// Miscellaneous getters.

static inline size_t usable_allocation(const binary_tree *const tree) {
  return tree->allocation - sizeof(binary_tree);
}

// An individual node's true allocation is counted as the size of its value and
// the size of a `bt_node`.
static inline size_t node_value_pair_size(const binary_tree *const tree) {
  return tree->value_size + sizeof(bt_node);
}

// This includes deleted and unallocated nodes.
static inline size_t cnt_nodes(const binary_tree *const tree) {
  return usable_allocation(tree) / node_value_pair_size(tree);
}

static inline size_t cnt_deleted_nodes(const binary_tree *const tree) {
  return tree->deleted_nodes->length;
}

static inline size_t cnt_unallocated_nodes(const binary_tree *const tree) {
  return cnt_nodes(tree) - tree->unallocated_region_index;
}

static inline size_t cnt_active_nodes(const binary_tree *const tree) {
  return cnt_nodes(tree) - cnt_deleted_nodes(tree) -
         cnt_unallocated_nodes(tree);
}

// Getters for specific extents of allocations.

static inline size_t calc_padding(const size_t values_alloc) {
  if (values_alloc == 0) return 0;
  return alignof(bt_node) - (values_alloc % alignof(bt_node));
}

static inline size_t values_allocation(const binary_tree *const tree) {
  return cnt_nodes(tree) * tree->value_size;
}

static inline size_t get_padding(const binary_tree *const tree) {
  return calc_padding(values_allocation(tree));
}

// Getters for specific regions of memory.

static inline byte *get_values(binary_tree *const tree) {
  return (byte *)(tree + 1);
}

static inline bt_node *get_nodes(binary_tree *const tree) {
  /*
   * Although `bt_node` has a greater alignment requirement than `byte` (the
   * type returned by `get_values()`), the casted value is guaranteed to be
   * aligned to the size of `bt_node` with the padding bytes. The cast to `void
   * *` is present to remove the associated linter warning.
   */
  return (void *)(get_values(tree) + values_allocation(tree) +
                  get_padding(tree));
}

// Getters for specific nodes and values.

static inline bt_node *node_from_index(binary_tree *const tree,
                                           const size_t index) {
  if (index == NULL_INDEX) return NULL;
  return get_nodes(tree) + index;
}

static inline void *value_from_index(binary_tree *const tree,
                                         const size_t index) {
  if (index == NULL_INDEX) return NULL;
  return get_values(tree) + (index * tree->value_size);
}

// Getter for indicies of nodes.

static inline size_t get_node_index(binary_tree *const tree,
                                    const bt_node *const node) {
  if (node == NULL) return NULL_INDEX;
  return (size_t)(node - get_nodes(tree));
}

// Getter for node values

static inline void *get_node_value(binary_tree *const tree,
                                   bt_node *const node) {
  if (node == NULL) return NULL;
  return value_from_index(tree, get_node_index(tree, node));
}

// Getters for nodes based on relative positions.

static inline bt_node *root_node(binary_tree *const tree) {
  if (tree->root_index == NULL_INDEX) return NULL;
  return node_from_index(tree, tree->root_index);
}

static inline bt_node *parent_node(binary_tree *const tree,
                                       const bt_node *const node) {
  if (node->parent_index == NULL_INDEX) return NULL;
  return node_from_index(tree, node->parent_index);
}

static inline bt_node *left_node(binary_tree *const tree,
                                     const bt_node *const node) {
  if (node->left_index == NULL_INDEX) return NULL;
  return node_from_index(tree, node->left_index);
}

static inline bt_node *right_node(binary_tree *const tree,
                                      const bt_node *const node) {
  if (node->right_index == NULL_INDEX) return NULL;
  return node_from_index(tree, node->right_index);
}

// - GETTERS END -

// - HELPER FUNCTIONS -

// Helper function used by `get_unalloc_node()`.
static inline bt_node *get_deleted_node(binary_tree *const tree) {
  const size_t *const node_index = (size_t *)stack_pop(tree->deleted_nodes);
  return (node_index == NULL) ? NULL : node_from_index(tree, *node_index);
}

// Helper function used by `bt_add_node()`.
static inline size_t leftmost_leaf_index(binary_tree *const tree) {
  const bt_node *candidate = root_node(tree);
  if (candidate != NULL) {
    while (candidate->left_index != NULL_INDEX)
      candidate = left_node(tree, candidate);
    return get_node_index(tree, candidate);
  }
  return NULL_INDEX;
}

// Helper function used by `bt_add_node()`.
static inline bt_node *get_unalloc_node(binary_tree *tree) {
  const size_t NODE_CNT = cnt_nodes(tree);
  if (cnt_nodes(tree) == cnt_active_nodes(tree)) return NULL;
  if (cnt_deleted_nodes(tree) != 0) return get_deleted_node(tree);
  if (tree->unallocated_region_index == NODE_CNT) return NULL;
  return get_nodes(tree) + (tree->unallocated_region_index++);
}

// Used by `bt_delete_node()` in tandem with `bt_traverse()`.
static inline bool register_deleted_node(binary_tree *const tree,
                                  bt_node *const node) {
  const size_t NODE_INDEX = get_node_index(tree, node);
  tree->deleted_nodes = stack_push(tree->deleted_nodes, &NODE_INDEX);
  return true;
}

// - HELPER FUNCTIONS END -

binary_tree *bt_add_node(binary_tree *tree, const void *const value) {
  if (cnt_nodes(tree) == cnt_active_nodes(tree)) {
    tree = bt_expand(tree);
    if (tree == NULL) return NULL;
  }

  bt_node *const node = get_unalloc_node(tree);
  memcpy(get_node_value(tree, node), value, tree->value_size);

  const size_t PARENT_INDEX = leftmost_leaf_index(tree);
  const size_t NODE_INDEX = get_node_index(tree, node);
  if (PARENT_INDEX != NULL_INDEX) {
    bt_node *const parent = node_from_index(tree, PARENT_INDEX);
    parent->left_index = NODE_INDEX;
    node->parent_index = PARENT_INDEX;
    node->left_index = node->right_index = NULL_INDEX;
  } else {
    tree->root_index = NODE_INDEX;
    node->parent_index = node->left_index = node->right_index = NULL_INDEX;
  }
  return tree;
}

inline void bt_delete(binary_tree *const tree) {
  if (tree->deleted_nodes != NULL) stack_delete(tree->deleted_nodes);
  free(tree);
}

void bt_delete_node(binary_tree *tree, bt_node *const node) {
  bt_node *const parent = parent_node(tree, node);
  if (parent != NULL) {
    if (parent->left_index == get_node_index(tree, node))
      parent->left_index = NULL_INDEX;
    else
      parent->right_index = NULL_INDEX;
  } else {
    tree->root_index = NULL_INDEX;
  }
  bt_traverse(tree, node, register_deleted_node);
}

binary_tree *bt_expand(binary_tree *const tree) {
  const size_t NUM_NODES = cnt_nodes(tree);
  if (NUM_NODES == 0) return bt_resize(tree, 1);
  return bt_resize(tree, EXPANSION_FACTOR * NUM_NODES);
}

binary_tree *bt_init(const size_t value_size, const size_t node_cnt) {
  const size_t NODES_ALLOC = sizeof(bt_node) * node_cnt;
  const size_t VALUES_ALLOC = value_size * node_cnt;
  const size_t PADDING = calc_padding(VALUES_ALLOC);
  const size_t TOTAL_ALLOC =
      NODES_ALLOC + VALUES_ALLOC + PADDING + sizeof(binary_tree);
  binary_tree *const tree = malloc(TOTAL_ALLOC);
  if (tree == NULL) return NULL;
  tree->deleted_nodes = stack_empty_new(node_cnt / 4, sizeof(size_t));
  tree->allocation = TOTAL_ALLOC;
  tree->root_index = NULL_INDEX;
  tree->value_size = value_size;
  tree->unallocated_region_index = 0;
  return tree;
}

binary_tree *bt_new_(const void *const data, const size_t value_size,
                     const size_t num_values) {
  binary_tree *const tree = bt_init(value_size, num_values);
  if (tree == NULL) return NULL;
  tree->unallocated_region_index = num_values;

  bt_node *const nodes = get_nodes(tree);
  if (num_values > 0) {
    tree->root_index = 0;
    bt_node *const root_node = nodes;
    root_node->parent_index = NULL_INDEX;
    root_node->left_index = NULL_INDEX;
    root_node->right_index = NULL_INDEX;
  }
  for (size_t i = 1; i < num_values; i++) {
    const size_t PARENT_INDEX = (i - 1) / 2;
    bt_node *const node = nodes + i;
    bt_node *const parent = nodes + PARENT_INDEX;
    node->parent_index = PARENT_INDEX;
    // Each new node should start without children.
    node->left_index = node->right_index = NULL_INDEX;
    /*
     * The line below is equivalent to:
     *  if (i & 1)
     *    parent->left_index = i;
     *  else
     *    parent->right_index = i;
     */
    *((size_t *)parent + 1 + !(i & 1)) = i;
  }
  memcpy(get_values(tree), data, value_size * num_values);
  return tree;
}

binary_tree *bt_resize(binary_tree *tree, const size_t new_capacity) {
  if (new_capacity < tree->unallocated_region_index) return tree;
  const size_t PAIR_SIZE = node_value_pair_size(tree);
  const size_t PADDING = calc_padding(tree->value_size * new_capacity);
  const size_t NEW_ALLOCATION =
      (PAIR_SIZE * new_capacity) + sizeof(binary_tree) + PADDING;
  {
    if (NEW_ALLOCATION == tree->allocation) return tree;
    binary_tree *const new_tree = realloc(tree, NEW_ALLOCATION);
    if (new_tree == NULL) return NULL;
    tree = new_tree;
  }
  if (new_capacity == 0) {
    tree->root_index = NULL_INDEX;
    tree->allocation = NEW_ALLOCATION;
    stack_reset(tree->deleted_nodes);
  } else {
    bt_node *const old_nodes_region = get_nodes(tree);
    tree->allocation = NEW_ALLOCATION;
    bt_node *const new_nodes_region = get_nodes(tree);
    const size_t MOVE_SIZE = sizeof(bt_node) * cnt_nodes(tree);
    memmove(new_nodes_region, old_nodes_region, MOVE_SIZE);
  }
  return tree;
}

void bt_swap_values(binary_tree *const tree, bt_node *const node_1,
                    bt_node *const node_2) {
  if (node_1 == node_2) return;
  const size_t VALUE_SIZE = tree->value_size;
  byte *restrict const value_1 = get_node_value(tree, node_1);
  byte *restrict const value_2 = get_node_value(tree, node_2);
  for (size_t i = 0; i < VALUE_SIZE; i++) {
    const byte temp = value_1[i];
    value_1[i] = value_2[i];
    value_2[i] = temp;
  }
}

/*
 * `bt_traverse()` will, by default, traverse first along the left branch,
 * deviating to a right branch if and only if the `left_index` of a traversed
 * node is `NULL_INDEX`. If both `left_index` and `right_index` are
 * `NULL_INDEX`, then `bt_traverse()` backtracks to the closest ancestor
 * node with a valid right branch that has not yet been traversed.
 *
 * Recursion is the natural solution to tree traversal, but recursion on a
 * particularly large tree could result in a stack overflow. Hence, we use a
 * stack (allocated on the heap) to keep track of divergent nodes in the tree.
 */
void bt_traverse(binary_tree *const tree, bt_node *const from,
                 bool (*const operation)(binary_tree *tree, bt_node *node)) {
  stack *divergent_nodes =
      stack_empty_new(cnt_active_nodes(tree), sizeof(bt_node **));

  bt_node *cur_node = from;
  bt_node *next_node = NULL;
  while (cur_node != NULL) {
    if (operation != NULL)
      if (!operation(tree, cur_node)) break;

    bt_node *const left = left_node(tree, cur_node);
    bt_node *const right = right_node(tree, cur_node);
    if (left != NULL) {
      /*
       * If both `left` and `right` are valid, continue down the left branch
       * and save `node` as a divergent node.
       *
       * Taking the address of `node` is necessary since `stack_push()`
       * expects the `elem` argument to be a pointer to data. Since `node` is
       * a pointer to a node, and we need to keep track of that pointer, we
       * take the address of that pointer.
       */
      if (right != NULL)
        divergent_nodes = stack_push(divergent_nodes, (void *)&cur_node);
      next_node = left;
    } else if (right != NULL) {
      next_node = right;
    } else {
      bt_node **const next_divergence = (bt_node **)stack_pop(divergent_nodes);
      // If there are no divergent nodes in the traversed path, we're done.
      if (next_divergence == NULL) break;
      next_node = right_node(tree, *next_divergence);
    }
    cur_node = next_node;
  }
  stack_delete(divergent_nodes);
}
