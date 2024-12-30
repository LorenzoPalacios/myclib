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

// - GETTERS BEGIN -

// Miscellaneous getters.

static inline size_t get_node_value_allocation(const binary_tree *const tree) {
  return tree->allocation - sizeof(binary_tree);
}

// An individual node's true allocation is counted as the size of its value and
// the size of a `bt_node`.
static inline size_t get_actual_node_size(const binary_tree *const tree) {
  return tree->value_size + sizeof(bt_node);
}

// This includes unallocated nodes.
static inline size_t get_total_num_nodes(const binary_tree *const tree) {
  return get_node_value_allocation(tree) / get_actual_node_size(tree);
}

// Getters for specific extents of allocations.

static inline size_t get_values_allocation(const binary_tree *const tree) {
  return get_total_num_nodes(tree) * tree->value_size;
}

static inline size_t get_nodes_allocation(const binary_tree *const tree) {
  return get_total_num_nodes(tree) * sizeof(bt_node);
}

static inline size_t calc_padding(const size_t values_alloc) {
  if (values_alloc == 0) return 0;
  return alignof(bt_node) - (values_alloc % alignof(bt_node));
}

static inline size_t get_padding(const binary_tree *const tree) {
  return calc_padding(get_values_allocation(tree));
}

// Getters for specific regions of memory.

static inline byte *get_values(binary_tree *const tree) {
  return (byte *)(tree + 1);
}

static inline bt_node *get_nodes(binary_tree *const tree) {
  /*
   * Although `bt_node` has a greater alignment requirement than `byte`, the
   * casted value is guaranteed to be aligned to the size of `bt_node` thanks to
   * the padding bytes. The cast to `void *` is present to remove the associated
   * linter warning.
   */
  return (void *)(get_values(tree) + get_values_allocation(tree) +
                  get_padding(tree));
}

// Getters for specific nodes and values.

static inline bt_node *get_node_from_index(binary_tree *const tree,
                                           const size_t index) {
  if (index == NULL_INDEX) return NULL;
  return get_nodes(tree) + index;
}

static inline byte *get_value_from_index(binary_tree *const tree,
                                         const size_t index) {
  if (index == NULL_INDEX) return NULL;
  return get_values(tree) + (index * tree->value_size);
}

// Getter for indicies of nodes.

static inline size_t get_node_index(binary_tree *const tree,
                                    const bt_node *const node) {
  return (size_t)(node - get_nodes(tree));
}

// Getters for nodes based on relative positions.

static inline bt_node *get_parent_node(binary_tree *const tree,
                                       const bt_node *const node) {
  if (node->parent_index == NULL_INDEX) return NULL;
  return get_node_from_index(tree, node->parent_index);
}

static inline bt_node *get_left_node(binary_tree *const tree,
                                     const bt_node *const node) {
  if (node->left_index == NULL_INDEX) return NULL;
  return get_node_from_index(tree, node->left_index);
}

static inline bt_node *get_right_node(binary_tree *const tree,
                                      const bt_node *const node) {
  if (node->right_index == NULL_INDEX) return NULL;
  return get_node_from_index(tree, node->right_index);
}

// - GETTERS END -

binary_tree *bt_init(const size_t value_size, const size_t node_cnt) {
  const size_t NODES_ALLOC = sizeof(bt_node) * node_cnt;
  const size_t VALUES_ALLOC = value_size * node_cnt;
  const size_t PADDING = calc_padding(VALUES_ALLOC);
  const size_t TOTAL_ALLOC =
      NODES_ALLOC + VALUES_ALLOC + PADDING + sizeof(binary_tree);
  binary_tree *const tree = malloc(TOTAL_ALLOC);
  if (tree == NULL) return NULL;

  tree->allocation = TOTAL_ALLOC;
  tree->root_index = NULL_INDEX;
  tree->unused_nodes = stack_empty_new(node_cnt / 4, sizeof(size_t));
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
  if (divergent_nodes == NULL) {
    const size_t NUM_NODES = get_total_num_nodes(tree);
    divergent_nodes = stack_empty_new(NUM_NODES, sizeof(bt_node **));
  } else {
    stack_reset(divergent_nodes);
  }

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
    if (operation != NULL)
      if (operation(tree, cur_node)) break;

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
    if (divergent_nodes->length > STK_CAP_SHRNK_THRES) low_stack_usage = false;
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

binary_tree *bt_resize(binary_tree *tree, const size_t new_capacity) {
  const size_t NODE_SIZE = get_actual_node_size(tree);
  const size_t ALLOCATION = (NODE_SIZE * new_capacity) + sizeof(binary_tree);
  {
    binary_tree *const new_tree = realloc(tree, ALLOCATION);
    if (new_tree == NULL) return NULL;
    tree = new_tree;
  }
  tree->allocation = ALLOCATION;
  if (new_capacity == 0) {
    tree->root_index = NULL_INDEX;
    stack_reset(tree->unused_nodes);
  }

  return tree;
}

binary_tree *bt_expand(binary_tree *const tree) {
  const size_t NUM_NODES = get_total_num_nodes(tree);
  if (NUM_NODES == 0) return bt_resize(tree, 1);
  return bt_resize(tree, EXPANSION_FACTOR * NUM_NODES);
}

void register_unalloc_node(binary_tree *const tree, bt_node *const open_node) {
  const size_t NODE_INDEX = get_node_index(tree, open_node);
  tree->unused_nodes = stack_push(tree->unused_nodes, &NODE_INDEX);
}

bt_node *get_unalloc_node(binary_tree *const tree) {
  const size_t *const node_index = (size_t *)stack_pop(tree->unused_nodes);
  return (node_index == NULL) ? NULL : get_node_from_index(tree, *node_index);
}

#include <stdio.h>

bool print_node(binary_tree *const tree, bt_node *const node) {
  printf("%hhd ", *(char *)get_value_from_index(tree, node->value_index));
  return false;
}

void print_tree_stats(binary_tree *const tree) {
  printf("# nodes: %zu\n", get_total_num_nodes(tree));
  printf("Nodes allocation: %zu\n", get_nodes_allocation(tree));
  printf("Values allocation: %zu\n", get_values_allocation(tree));
  printf("Padding: %zu\n", get_padding(tree));
}

int main(void) {
  const char data[] = {7, 4, 9, 3, 0};
  binary_tree *tree = bt_new(data);
  if (tree == NULL) return EXIT_FAILURE;

  bt_traverse(tree, get_node_from_index(tree, tree->root_index), print_node);
  putchar('\n');
  print_tree_stats(tree);

  tree = bt_resize(tree, 0);
  print_tree_stats(tree);

  tree = bt_expand(tree);
  print_tree_stats(tree);

  bt_delete(tree);
  return EXIT_SUCCESS;
}
