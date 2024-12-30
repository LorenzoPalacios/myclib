#include "binarytree.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

#include "../../stack/stack.h"

// Reallocation factor used by `bt_expand()`.
#define EXPANSION_FACTOR (2)

static inline byte *get_values(binary_tree *const tree) {
  return (byte *)(tree + 1);
}

static inline bt_node *get_nodes(binary_tree *const tree) {
  return (bt_node *)(get_values(tree) + tree->padding);
}

static inline bt_node *get_node_from_index(binary_tree *const tree,
                                           const size_t index) {
  if (index == NULL_INDEX) return NULL;
  return get_nodes(tree) + index;
}

static inline byte *get_value_from_index(binary_tree *const tree,
                                         const size_t index) {
  return get_values(tree) + (index * tree->value_size);
}

static inline size_t get_node_index(binary_tree *const tree,
                                    const bt_node *const node) {
  return (size_t)(node - get_nodes(tree));
}

static inline bt_node *get_parent_node(binary_tree *const tree,
                                       const bt_node *const node) {
  return get_node_from_index(tree, node->parent_index);
}

static inline bt_node *get_left_node(binary_tree *const tree,
                                     const bt_node *const node) {
  return get_node_from_index(tree, node->left_index);
}

static inline bt_node *get_right_node(binary_tree *const tree,
                                      const bt_node *const node) {
  return get_node_from_index(tree, node->right_index);
}

binary_tree *bt_init(const size_t value_size, const size_t node_cnt) {
  const size_t NODES_ALLOC = sizeof(bt_node) * node_cnt;
  const size_t VALUES_ALLOC = value_size * node_cnt;
  const size_t PADDING = VALUES_ALLOC % alignof(bt_node);
  const size_t TOTAL_ALLOC =
      NODES_ALLOC + VALUES_ALLOC + PADDING + sizeof(binary_tree);
  binary_tree *const tree = malloc(TOTAL_ALLOC);
  if (tree == NULL) return NULL;

  tree->allocation = TOTAL_ALLOC;
  tree->padding = PADDING;
  tree->root_index = NULL_INDEX;
  tree->unused_nodes = stack_empty_new(1, sizeof(bt_node *));
  tree->value_size = value_size;
  return tree;
}

binary_tree *bt_new_(const void *const data, const size_t elem_size,
                     const size_t num_elems) {
  binary_tree *const tree = bt_init(elem_size, num_elems);

  if (num_elems > 0) {
    tree->root_index = 0;
    bt_node *const root_node = get_node_from_index(tree, tree->root_index);
    byte *const root_value = get_value_from_index(tree, tree->root_index);
    root_node->parent_index = NULL_INDEX;
    root_node->left_index = NULL_INDEX;
    root_node->right_index = NULL_INDEX;
    root_node->value_index = tree->root_index;
    memcpy(root_value, data, elem_size);
  }

  for (size_t i = 1; i < num_elems; i++) {
    bt_node *const node = get_node_from_index(tree, i);
    byte *const value = get_value_from_index(tree, i);

    node->value_index = i;
    node->parent_index = i / 2;
    // Each new node should have no children from the outset.
    node->left_index = node->right_index = NULL_INDEX;

    bt_node *const parent_node = get_parent_node(tree, node);
    // Potential optimization to replace conditional:
    // *((size_t *)parent_node + 1 + (i & 1)) = i;
    if (i & 1)
      parent_node->left_index = i;
    else
      parent_node->right_index = i;
    memcpy(value, (const byte *)data + (i * elem_size), elem_size);
  }
  return tree;
}

void bt_delete_(binary_tree **const tree) {
  free(*tree);
  *tree = NULL;
}

void bt_delete_s_(binary_tree **const tree) {
  memset(*tree, 0, (*tree)->allocation);
  bt_delete_(tree);
}

/*
 * `bt_traverse()` will, by default, traverse first along the left branch,
 * deviating to a right branch if and only if the `left_index` of a traversed
 * node is `NULL_INDEX`. If both `left_index` and `right_index` are
 * `NULL_INDEX`, then we backtrack to the most recent ancestor node with a right
 * branch that has not yet been traversed.
 *
 * Note that these ancestor nodes will have already had their left branches
 * traversed prior to any backtracking, hence why we can always select their
 * right branch.
 *
 * Recursion is the natural solution to tree traversal, but recursion on a
 * particularly large tree could result in a stack overflow. Hence, we use a
 * stack (allocated on the heap) to keep track of divergent nodes in the tree.
 */
void bt_traverse(binary_tree *const tree, bt_node *const from,
                 bool (*const operation)(binary_tree *tree, bt_node *node)) {
  static stack *divergent_nodes = NULL;

  // If the stack is uninitialized, initialize it.
  // Otherwise, ensure the stack does not contain any divergent nodes from prior
  // searches by clearing its contents.
  if (divergent_nodes == NULL)
    divergent_nodes = stack_empty_new(12, sizeof(bt_node **));
  else
    stack_reset(divergent_nodes);

  /*
   * If this value reaches `TRAVERSAL_STACK_SHRINK_COUNTER_MAX`,
   * `divergent_nodes` will have its allocation shrunk.
   *
   * This counter will reset if the `divergent_nodes->used_capacity` comes
   * within `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT` of
   * `divergent_nodes->capacity`.
   */
  static size_t stack_shrink_counter = 0;
  const size_t STK_CAP_SHRNK_THRES =
      (size_t)(TRAVERSAL_STACK_MAJOR_USAGE_PERCENT *
               stack_capacity(divergent_nodes));

  bool low_stack_usage = true;
  bt_node *cur_node = from;
  bt_node *next_node = NULL;
  while (cur_node != NULL) {
    if (operation != NULL) operation(tree, cur_node);
    if (stop_condition != NULL)
      if (stop_condition(tree, cur_node)) break;
    bt_node *const left = get_left_node(tree, cur_node);
    bt_node *const right = get_right_node(tree, cur_node);
    if (left != NULL) {
      /*
       * If both `left` and `right` are valid, continue down the left branch and
       * save `node` as a divergent node.
       *
       * Taking the address of `node` is necessary since `stack_push()` expects
       * the `elem` argument to be a pointer to data. Since `node` is a pointer
       * to a node, and we need to keep track of that pointer, we take the
       * address of that pointer.
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
      next_node = get_right_node(tree, *next_divergence);
    }
    // Checking if the stack is using a majority of its capacity.
    if (divergent_nodes->length > STK_CAP_SHRNK_THRES)
      low_stack_usage = false;
    cur_node = next_node;
  }
  if (low_stack_usage)
    stack_shrink_counter++;
  else
    stack_shrink_counter = 0;

  if (stack_shrink_counter == TRAVERSAL_STACK_SHRINK_COUNTER_MAX) {
    stack_resize(divergent_nodes, divergent_nodes->allocation / 2);
    stack_shrink_counter = 0;
  }
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
binary_tree *bt_resize(binary_tree *tree, size_t new_size) {
  {
    const size_t NODE_SIZE = tree->value_size + sizeof(bt_node);
    const size_t SIZE_DIFF = new_size % NODE_SIZE;
    if (SIZE_DIFF != 0) new_size += NODE_SIZE - SIZE_DIFF;
  }
  if (new_size < tree->used_allocation) {
    const size_t NODE_SIZE = tree->value_size + sizeof(bt_node);
    tree->used_allocation = new_size;
  }
  tree = realloc(tree, new_size);
  if (tree == NULL) return NULL;
  tree->allocation = new_size;

  return tree;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
binary_tree *resize_tree_s(binary_tree *tree, const size_t new_size) {
  return realloc(tree, new_size);
}

binary_tree *bt_expand(binary_tree *const tree) {
  return bt_resize(tree, tree->allocation * BT_EXPANSION_FACTOR);
}

void register_unalloc_node(binary_tree *const tree, bt_node *const open_node) {
  tree->unused_nodes = stack_push(tree->unused_nodes, &open_node);
}

/* write documentation */
bt_node *get_unalloc_node(binary_tree *const tree) {
  bt_node **const unalloc_node = stack_pop(tree->unused_nodes);
  return (unalloc_node == NULL) ? NULL : *unalloc_node;
}

/* Helper function used by `bt_get_open_leaf()`. */
static bool node_has_open_child(void *const tree_and_node) {
  const bt_node *const node = *(bt_node **)tree_and_node;
  return node->left == NULL || node->right == NULL;
}

/*
 * Helper function used by `bt_get_open_leaf()`.
 *
 * \returns A pointer to a pointer to an open child node.
 */
static void *ret_node_with_open_child(void *const tree_and_node) {
  bt_node *const candidate = *(bt_node **)tree_and_node;
  if (node_has_open_child(candidate)) return candidate;
  return NULL;
}

bt_node *bt_get_open_leaf(bt_node *const origin) {
  return bt_traverse(NULL, origin, ret_node_with_open_child,
                     node_has_open_child);
}

void *get_node_value(bt_node *const node) { return }

/*
 * There's likely a better implementation to check whether or not two nodes
 * exist within the same tree. For now, this will do.
 */
bool do_nodes_coexist_in_tree(const bt_node *const node_1,
                              const bt_node *const node_2) {
  if (node_1 == node_2) return true;
  if (node_1->parent == node_2->parent) return true;

  const bt_node *node_1_root_node = NULL;
  {
    const bt_node *ancestor = node_1->parent;
    while (ancestor != NULL) {
      /*
       * If `node_2` is encountered while traversing the ancestry of `node_1`,
       * then we know they coexist in a tree.
       */
      if (ancestor == node_2) return true;
      ancestor = ancestor->parent;
    }
  }

  const bt_node *node_2_root_node = NULL;
  {
    const bt_node *ancestor = node_2->parent;
    while (ancestor != NULL) {
      /*
       * If `node_1` is encountered while traversing the ancestry of `node_2`,
       * then we know they coexist in a tree.
       */
      if (ancestor == node_1) return true;
      ancestor = ancestor->parent;
    }
  }
  /*
   * If both `node_1` and `node_2` have no root node and are not equivalent to
   * one another, then either one or both nodes are a root node for a tree or
   * a freestanding node.
   */
  if (node_1_root_node == NULL && node_2_root_node == NULL) return false;
  /*
   * Otherwise, if the nodes share a common root node, then they are within the
   * same tree.
   */
  return node_1_root_node == node_2_root_node;
}

bt_node **get_open_child_in_node(bt_node *const node) {
  if (node->left == NULL) return &node->left;
  if (node->right == NULL) return &node->right;
  return NULL;
}

/*
 * Helper function used by functions whose purpose is to add nodes to a tree,
 * such as `add_freestanding_node()`.
 *
 * If possible, this function shifts the memory region dedicated to nodes to
 * the right by `tree->value_size` bytes.
 *
 * \return A pointer to a slot of unused memory in `tree` whose capacity is
 * equal to `tree->value_size`.
 * \note `tree` must have enough space to accomodate a new value. If this
 * condition is not met, the behavior is undefined.
 */
static void *make_room_for_new_value(binary_tree *const tree) {
  const size_t NUM_NODES = get_num_nodes_in_tree(tree);
  /* Increment past the tree header, then increment past the nodes. */
  void *const NODES_BEGIN = (byte *)(tree + 1) + NUM_NODES * tree->value_size;
  size_t SHIFT_MAGNITUDE = tree->value_size * alignof(bt_node) + 1;
  if (tree->padding != 0) {
    const size_t diff = tree->value_size % alignof(bt_node);
    SHIFT_MAGNITUDE -=
        (tree->padding + diff) / alignof(bt_node) * alignof(bt_node);
  }

  void *const NEW_NODES_BEGIN = (byte *)NODES_BEGIN + SHIFT_MAGNITUDE;
  /*
   * The number of values should correspond to the number of nodes, hence why
   * the length argument is `NUM_NODES`.
   */
  memmove(NEW_NODES_BEGIN, NODES_BEGIN, NUM_NODES);
  return NODES_BEGIN;
}

static void *get_end_of_values_region(binary_tree *const tree) {
  const size_t NUM_NODES = get_num_nodes_in_tree(tree);
  const size_t VALUES_ALLOC = tree->value_size * NUM_NODES;
  return (byte *)(tree + 1) + VALUES_ALLOC;
}

static bt_node *get_end_of_nodes_region(binary_tree *const tree) {
  const size_t NUM_NODES = get_num_nodes_in_tree(tree);
  const size_t NODES_ALLOC = sizeof(bt_node) * NUM_NODES;
  const void *const VALUES_REGION_END = get_end_of_values_region(tree);
  return (void *)((byte *)VALUES_REGION_END + tree->padding + NODES_ALLOC);
}

static void incorporate_node(binary_tree *const tree, bt_node *const node) {
  bt_node *const node_parent = bt_get_open_leaf(tree->root);
  if (node_parent->left == NULL)
    node_parent->left = node;
  else
    node_parent->right = node;
  node->parent = node_parent;
}

binary_tree *add_freestanding_node(binary_tree *tree, bt_node **const node) {
  const bt_node *node_actual = *node;
  if (node_actual == NULL) return tree;

  bt_node *open_node;
  if (tree->unused_nodes->length != 0)
    /* Use any unallocated nodes in `tree` before reallocating. */
    open_node = get_unalloc_node(tree);
  else {
    tree = bt_expand(tree);
    void *const open_val = make_room_for_new_value(tree);
    open_node = get_end_of_nodes_region(tree);
    open_node->value = open_val;
  }
  incorporate_node(tree, open_node);
  memcpy(open_node->value, node_actual->value, tree->value_size);
  *node = open_node;
  return tree;
}

bt_node *new_bt_node(const void *value, size_t value_size) {
  bt_node *const new_node = malloc(sizeof(bt_node) + value_size);
  if (new_node == NULL) return NULL;

  new_node->value = new_node + 1;
  new_node->parent = new_node->left = new_node->right = NULL;
  memcpy(new_node->value, value, value_size);
  return new_node;
}

static void *print_node(void *const tree_and_node) {
  const bt_node *const node = *(bt_node **)tree_and_node;
  printf("%d\n", *(int *)node->value);
  return NULL;
}
