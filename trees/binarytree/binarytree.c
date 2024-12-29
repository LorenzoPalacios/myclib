#include "binarytree.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

// Stacks are used to track unallocated nodes in trees and in
// `traverse_from()` to avoid the usage of recursion.
#include "../../stack/stack.h"

// Reallocation factor used by `expand_binary_tree()`.
#define EXPANSION_FACTOR (2)

binary_tree *new_binary_tree_(const void *const data, const size_t elem_size,
                              const size_t num_elems) {
  binary_tree *tree;
  const size_t NODE_VALUE_PADDING = (elem_size * num_elems) % alignof(node_bt);
  // This region is enclosed in its own block to avoid autocomplete pollution
  // later during tree construction.
  {
    const size_t NODE_SIZE = sizeof(node_bt) + elem_size;
    const size_t TOTAL_REQ_MEM =
        num_elems * NODE_SIZE + sizeof(binary_tree) + NODE_VALUE_PADDING;
    tree = malloc(TOTAL_REQ_MEM); /* TREE ALLOCATED HERE */
    if (tree == NULL) return NULL;

    tree->value_size = elem_size;
    tree->unused_nodes = stack_empty_new(1, sizeof(node_bt **));
    tree->padding = NODE_VALUE_PADDING;
  }
  /* The values will be stored directly after the tree header. */
  byte *const values_region_start = (void *)(tree + 1);
  /*
   * The nodes will be stored directly after the region of memory dedictaed to
   * storing each node's values.
   * Originally, the nodes were stored after the tree header and the values
   * stored after the nodes, but it is actually easier to have the values
   * stored before the nodes when adding nodes to a tree.
   */
  node_bt *const nodes_region_start =
      (void *)(values_region_start + num_elems * elem_size +
               NODE_VALUE_PADDING);
  for (size_t i = 0; i < num_elems; i++) {
    node_bt *const cur_node = nodes_region_start + i;
    const size_t value_offset = i * elem_size;
    byte *const cur_node_value = values_region_start + value_offset;
    node_bt *parent_node;
    /* The root node has no parent. */
    if (i == 0)
      parent_node = NULL;
    else {
      /* Ensures each node receives at most 2 child nodes. */
      parent_node = cur_node - (i / 2 + 1);
      if (i & 1)
        parent_node->left = cur_node;
      else
        parent_node->right = cur_node;
    }
    memcpy(cur_node_value, (byte *)data + i * elem_size, elem_size);
    cur_node->value_offset = value_offset;
    cur_node->parent = parent_node;
    /* Each new node should have no children from the outset. */
    cur_node->left = cur_node->right = NULL;
  }
  tree->root = nodes_region_start; /* The first node is always the root node. */
  return tree;
}

void delete_binary_tree(binary_tree **const tree) {
  free(*tree);
  *tree = NULL;
}

void delete_binary_tree_s(binary_tree **const tree) {
  memset(*tree, 0, (*tree)->allocation);
  delete_binary_tree(tree);
}

/* NEEDS REWRITE */
node_bt *remove_node_from_tree(binary_tree *const tree, node_bt *const target) {
  node_bt *const node_copy = malloc(tree->value_size);
  if (node_copy == NULL) return NULL;
  memcpy(node_copy, target, tree->value_size);
  // delete_node_from_tree(tree, target);
  node_copy->parent = NULL;
  return node_copy;
}

/* REWRITE TO USE TRAVERSE_DESCENDANTS() */
size_t left_branch_depth(const node_bt *const origin) {
  size_t depth = 0;
  for (node_bt *cur_node = origin->left; cur_node != NULL; depth++) {
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }
  return depth;
}

/* REWRITE TO USE TRAVERSE_DESCENDANTS() */
size_t right_branch_depth(const node_bt *const origin) {
  size_t depth = 0;
  for (node_bt *cur_node = origin->right; cur_node != NULL; depth++) {
    if (cur_node->right == NULL)
      cur_node = cur_node->left;
    else
      cur_node = cur_node->right;
  }
  return depth;
}

size_t get_depth(const node_bt *const origin) {
  const size_t left_depth = left_branch_depth(origin);
  const size_t right_depth = right_branch_depth(origin);
  return (left_depth > right_depth ? left_depth : right_depth);
}

/*
 * Helper function for `count_descendant_nodes()` to be used as `op` for
 * `traverse_from()`.
 *
 * \return A pointer to a `size_t` which describes the number of nodes
 * traversed.
 */
static void *count_node(void *tree_and_cur_node) {
  const node_bt *const cur_node = *(node_bt **)tree_and_cur_node;
  static size_t count = 0;
  count++;
  /*
   * `traverse_from()` will not pass a `NULL` pointer to this function,
   * so we can pass `NULL` from `count_descendant_nodes()` to tell this function
   * to reset `count`.
   */
  if (cur_node == NULL) count = 0;
  return &count;
}

size_t count_descendant_nodes(node_bt *const origin) {
  /*
   * Since `traverse_from()` returns the last returned value of `op`,
   * which is `count_node()`, it will return a pointer to the number of
   * traversed nodes. Then, we subtract one, since `traverse_from()` will count
   * `origin` when we only need to count the descendants of `origin`.
   */
  const size_t count =
      *(size_t *)traverse_from(NULL, origin, count_node, NULL) - 1;
  count_node(NULL); /* Tells `count_node()` to reset its counter. */
  return count;
}

/*
 * Helper function for `delete_node()` to be used as `op` for `traverse_from()`.
 * Returns the current number of deleted nodes, which can be reset by passing
 * `NULL` as `tree_and_cur_node`.
 *
 * \return A pointer to a `size_t` which describes the number of nodes
 * traversed.
 */
static void *mark_node_as_unalloc(void *tree_and_cur_node) {
  static size_t unalloc_count = 0;

  binary_tree *tree = tree_and_cur_node;
  node_bt *target_node = *(node_bt **)tree_and_cur_node;
  /*
   * Like with `count_node()`, since `traverse_from()` will not return `NULL`
   * (`delete_node()` passes a tree and an origin node to `traverse_from()`), we
   * can call this function from `delete_node()` or similar and pass `NULL` as
   * its argument to signal that the counter should be reset.
   */
  if (tree == NULL || target_node == NULL) {
    unalloc_count = 0;
  } else {
    register_unalloc_node(tree, target_node);
    unalloc_count++;
  }
  return &unalloc_count;
}

size_t get_num_nodes_in_tree(const binary_tree *const tree) {
  const size_t NODES_ALLOCATION = tree->used_allocation - sizeof(binary_tree);
  const size_t NODE_SIZE = tree->value_size + sizeof(node_bt);
  return NODES_ALLOCATION / NODE_SIZE;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
void delete_node(binary_tree *const tree, node_bt *target) {
  const size_t TOTAL_NODE_SIZE = sizeof(node_bt) + tree->value_size;
  if (target == tree->root) {
    tree->root = NULL;
    return;
  }
  /*
   * Remove any linkage from the parent of `target` and `target`. We can leave
   * `target` itself unmodified since any access to it after this function is
   * completed technically invokes undefined behavior.
   */
  if (target->parent->left == target)
    target->parent->left = NULL;
  else
    target->parent->right = NULL;

  /*
   * `mark_node_as_unalloc()` returns a pointer to `size_t` which describes the
   * number of nodes that have been marked unallocated.
   */
  const size_t DELETED_NODES = *(size_t *)mark_node_as_unalloc(target);
  /* Signaling `mark_node_as_unalloc()` to reset its counter. */
  mark_node_as_unalloc(NULL);
  tree->used_allocation -= DELETED_NODES * tree->value_size;
}

node_bt *next_ancestral_divergence(const node_bt *const origin) {
  node_bt *cur_node = origin->parent;
  while (cur_node != NULL) {
    if (cur_node->left != NULL && cur_node->right != NULL) return cur_node;
    cur_node = cur_node->parent;
  }
  return NULL;
}

#include <stdio.h>

void *traverse_from(binary_tree *tree, node_bt *const origin,
                    void *(*const op)(void *tree_and_cur_node),
                    bool (*const stop_condition)(void *tree_and_cur_node)) {
  /*
   * The function will search along the left branch of `origin` and will deviate
   * to a right branch if and only if the `left` pointer of a traversed node is
   * `NULL`. If both `left` and `right` are `NULL`, then we backtrack to the
   * last divergent node that has not yet been traversed.
   *
   * Note that these divergent nodes will have already had their left branches
   * traversed prior to any backtracking, hence why we can always select their
   * right branch.
   *
   * Recursion is the natural solution to tree traversal, but recursion on a
   * particularly large tree could blow up the stack, hence why it isn't used
   * here.
   */
  static stack *divergent_nodes = NULL;
  /*
   * If the stack is uninitialized, initialize it.
   * Otherwise, ensure the stack does not contain any divergent nodes from prior
   * searches by clearing its contents.
   */
  if (divergent_nodes == NULL)
    divergent_nodes = new_empty_stack(12, sizeof(node_bt **));
  else
    clear_stack(divergent_nodes);

  /*
   * If this value reaches `TRAVERSAL_STACK_SHRINK_COUNTER_MAX`,
   * `divergent_nodes` will have its allocation shrunk.
   *
   * This counter will reset if the `divergent_nodes->used_capacity` comes
   * within `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT` of
   * `divergent_nodes->capacity`.
   */
  static byte stack_shrink_counter = 0;
  const size_t STK_CAP_SHRNK_THRES =
      TRAVERSAL_STACK_MAJOR_USAGE_PERCENT * divergent_nodes->capacity;

  byte arg_buf[sizeof(binary_tree *) + sizeof(node_bt *)];
  /*
   * Denotes the position in `arg_buf` where the pointer, `cur_node`, will be
   * written. This will be the first value in the passed array.
   */
  node_bt **const arg_buf_pos_cur_node = (void *)(arg_buf);

  /*
   * Denotes the position in `arg_buf` where the pointer, `tree`, will be
   * written. This will be the second value in the passed array.
   */
  binary_tree **const arg_buf_pos_tree = (void *)(arg_buf + sizeof(node_bt *));
  /* `tree` should be a constant value, so we assign it here. */
  *arg_buf_pos_tree = tree;

  bool low_stack_usage = true;
  void *ret_val = NULL;
  node_bt *cur_node = origin;
  node_bt *next_node = NULL;
  while (cur_node != NULL) {
    /* Write the pointer that is `cur_node` to the argument buffer. */
    *arg_buf_pos_cur_node = cur_node;
    if (op != NULL) ret_val = op(arg_buf);
    if (stop_condition != NULL)
      if (stop_condition(arg_buf)) break;
    if (cur_node->left != NULL) {
      /*
       * If both `left` and `right` are valid, continue down the left branch and
       * save `cur_node` as a divergent node.
       *
       * Taking the address of `cur_node` is necessary since `stack_push()`
       * expects the `elem` argument to be a pointer to data. Since `cur_node`
       * is a pointer to a node, and we need to keep track of that pointer, we
       * take the address of that pointer.
       */
      if (cur_node->right != NULL)
        divergent_nodes = stack_push(divergent_nodes, &cur_node);
      next_node = cur_node->left;
    } else if (cur_node->right != NULL) {
      next_node = cur_node->right;
    } else {
      node_bt **const next_divergence = stack_pop(divergent_nodes);
      /* If there are no divergent nodes in the traversed path, we're done. */
      if (next_divergence == NULL) break;
      next_node = (*next_divergence)->right;
    }
    /* Checking if the stack is using a majority of its capacity. */
    if (divergent_nodes->used_capacity > STK_CAP_SHRNK_THRES)
      low_stack_usage = false;
    /* Continuing to the next node. */
    cur_node = next_node;
  }
  if (low_stack_usage)
    stack_shrink_counter++;
  else
    stack_shrink_counter = 0;

  if (stack_shrink_counter == TRAVERSAL_STACK_SHRINK_COUNTER_MAX) {
    if (divergent_nodes->capacity > divergent_nodes->elem_size)
      divergent_nodes =
          resize_stack(divergent_nodes, divergent_nodes->capacity / 2);
    stack_shrink_counter = 0;
  }
  return ret_val;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
binary_tree *resize_tree(binary_tree *tree, size_t new_size) {
  {
    const size_t NODE_SIZE = tree->value_size + sizeof(node_bt);
    const size_t SIZE_DIFF = new_size % NODE_SIZE;
    if (SIZE_DIFF != 0) new_size += NODE_SIZE - SIZE_DIFF;
  }
  if (new_size < tree->used_allocation) {
    const size_t NODE_SIZE = tree->value_size + sizeof(node_bt);
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

binary_tree *expand_binary_tree(binary_tree *const tree) {
  return resize_tree(tree, tree->allocation * BT_EXPANSION_FACTOR);
}

void register_unalloc_node(binary_tree *const tree, node_bt *const open_node) {
  tree->unused_nodes = stack_push(tree->unused_nodes, &open_node);
}

/* write documentation */
node_bt *get_unalloc_node(binary_tree *const tree) {
  node_bt **const unalloc_node = stack_pop(tree->unused_nodes);
  return (unalloc_node == NULL) ? NULL : *unalloc_node;
}

/* Helper function used by `find_open_descendant()`. */
static bool node_has_open_child(void *const tree_and_node) {
  const node_bt *const node = *(node_bt **)tree_and_node;
  return node->left == NULL || node->right == NULL;
}

/*
 * Helper function used by `find_open_descendant()`.
 *
 * \returns A pointer to a pointer to an open child node.
 */
static void *ret_node_with_open_child(void *const tree_and_node) {
  node_bt *const candidate = *(node_bt **)tree_and_node;
  if (node_has_open_child(candidate)) return candidate;
  return NULL;
}

node_bt *find_open_descendant(node_bt *const origin) {
  return traverse_from(NULL, origin, ret_node_with_open_child,
                       node_has_open_child);
}

void *get_node_value(node_bt *const node) { return }

/*
 * There's likely a better implementation to check whether or not two nodes
 * exist within the same tree. For now, this will do.
 */
bool do_nodes_coexist_in_tree(const node_bt *const node_1,
                              const node_bt *const node_2) {
  if (node_1 == node_2) return true;
  if (node_1->parent == node_2->parent) return true;

  const node_bt *node_1_root_node = NULL;
  {
    const node_bt *ancestor = node_1->parent;
    while (ancestor != NULL) {
      /*
       * If `node_2` is encountered while traversing the ancestry of `node_1`,
       * then we know they coexist in a tree.
       */
      if (ancestor == node_2) return true;
      ancestor = ancestor->parent;
    }
  }

  const node_bt *node_2_root_node = NULL;
  {
    const node_bt *ancestor = node_2->parent;
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

node_bt **get_open_child_in_node(node_bt *const node) {
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
  size_t SHIFT_MAGNITUDE = tree->value_size * alignof(node_bt) + 1;
  if (tree->padding != 0) {
    const size_t diff = tree->value_size % alignof(node_bt);
    SHIFT_MAGNITUDE -=
        (tree->padding + diff) / alignof(node_bt) * alignof(node_bt);
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

static node_bt *get_end_of_nodes_region(binary_tree *const tree) {
  const size_t NUM_NODES = get_num_nodes_in_tree(tree);
  const size_t NODES_ALLOC = sizeof(node_bt) * NUM_NODES;
  const void *const VALUES_REGION_END = get_end_of_values_region(tree);
  return (void *)((byte *)VALUES_REGION_END + tree->padding + NODES_ALLOC);
}

static void incorporate_node(binary_tree *const tree, node_bt *const node) {
  node_bt *const node_parent = find_open_descendant(tree->root);
  if (node_parent->left == NULL)
    node_parent->left = node;
  else
    node_parent->right = node;
  node->parent = node_parent;
}

binary_tree *add_freestanding_node(binary_tree *tree, node_bt **const node) {
  const node_bt *node_actual = *node;
  if (node_actual == NULL) return tree;

  node_bt *open_node;
  if (tree->unused_nodes->length != 0)
    /* Use any unallocated nodes in `tree` before reallocating. */
    open_node = get_unalloc_node(tree);
  else {
    tree = expand_binary_tree(tree);
    void *const open_val = make_room_for_new_value(tree);
    open_node = get_end_of_nodes_region(tree);
    open_node->value = open_val;
  }
  incorporate_node(tree, open_node);
  memcpy(open_node->value, node_actual->value, tree->value_size);
  *node = open_node;
  return tree;
}

node_bt *new_bt_node(const void *value, size_t value_size) {
  node_bt *const new_node = malloc(sizeof(node_bt) + value_size);
  if (new_node == NULL) return NULL;

  new_node->value = new_node + 1;
  new_node->parent = new_node->left = new_node->right = NULL;
  memcpy(new_node->value, value, value_size);
  return new_node;
}

static void *print_node(void *const tree_and_node) {
  const node_bt *const node = *(node_bt **)tree_and_node;
  printf("%d\n", *(int *)node->value);
  return NULL;
}

#include <time.h>

int main(void) {
  int data[0x1000];
  srand(time(NULL));
  for (size_t i = 0; i < 0x1000; i++) data[i] = i;

  binary_tree *tree = new_binary_tree(data);
  node_bt *random_node = new_bt_node(data + 1, sizeof(*data));
  tree = add_freestanding_node(tree, &random_node);
  traverse_from(NULL, random_node, print_node, NULL);
  delete_tree(tree);

  return 0;
}
