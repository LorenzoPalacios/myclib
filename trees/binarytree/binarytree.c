#include "binarytree.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * Stacks are used to track unallocated nodes in trees and in
 * `traverse_from()` to avoid the usage of recursion.
 */
#include "../../stack/stack.h"
#include "../trees.h"

typedef unsigned char byte;

/* Reallocation factor used by `expand_binary_tree()`. */
#define BT_REALLOC_FACTOR (2)

binary_tree *_new_binary_tree(const void *const data, const size_t elem_size,
                              const size_t num_elems) {
  binary_tree *tree;
  /*
   * This region is enclosed in its own block to avoid autocomplete pollution
   * later during tree construction.
   */
  {
    const size_t NODE_SIZE = sizeof(node_bt) + elem_size;
    const size_t UNALLOC_NODES_STK_SIZE = sizeof(stack) + sizeof(node_bt **);
    const size_t TOTAL_REQ_MEM =
        num_elems * NODE_SIZE + UNALLOC_NODES_STK_SIZE + sizeof(binary_tree);
    tree = malloc(TOTAL_REQ_MEM); /* TREE ALLOCATED HERE */
    if (tree == NULL) return NULL;

    tree->value_size = elem_size;
    tree->used_allocation = tree->allocation = TOTAL_REQ_MEM;
    /*
     * The stack used for tracking unallocated nodes should exist as the last
     * object in the allocated region for the overall tree.
     */
    tree->unused_nodes =
        (void *)((byte *)tree + TOTAL_REQ_MEM - UNALLOC_NODES_STK_SIZE);
    stack *const unused_nodes = tree->unused_nodes;
    unused_nodes->data = unused_nodes + 1;
    unused_nodes->capacity = UNALLOC_NODES_STK_SIZE - sizeof(stack);
    unused_nodes->used_capacity = 0;
    unused_nodes->elem_size = sizeof(node_bt **);
    unused_nodes->length = 0;
  }

  /* The nodes will be stored directly after the tree header. */
  node_bt *const nodes_region_start = (void *)(tree + 1);
  /*
   * The values will be stored directly after the region of memory allocated for
   * the nodes.
   * Originally, the value for each node was stored in the memory directly after
   * their associated node. This was changed to avoid alignment issues.
   */
  byte *const values_region_start = (byte *)(nodes_region_start + num_elems);
  for (size_t i = 0; i < num_elems; i++) {
    node_bt *const cur_node = nodes_region_start + i;
    byte *const cur_node_value = values_region_start + i * elem_size;
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
    cur_node->value = cur_node_value;
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
  delete_node_from_tree(tree, target);
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
  const node_bt *const cur_node =
      (void *)((binary_tree *)tree_and_cur_node + 1);
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
  node_bt *target_node = (void *)((binary_tree *)tree_and_cur_node + 1);
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
  const size_t TREE_HEADER_SIZE = sizeof(binary_tree);
  const size_t UNUSED_NODES_SIZE = tree->unused_nodes->capacity + sizeof(stack);
  const size_t NON_NODES_ALLOCATION = TREE_HEADER_SIZE + UNUSED_NODES_SIZE;

  const size_t NODES_ALLOCATION = tree->used_allocation - NON_NODES_ALLOCATION;
  const size_t NODE_SIZE = tree->value_size + sizeof(node_bt);

  return NODES_ALLOCATION / NODE_SIZE;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
void delete_node(binary_tree *const tree, node_bt *target) {
  const size_t TOTAL_NODE_SIZE = sizeof(node_bt) + tree->value_size;
  if (target == tree->root) {
    tree->used_allocation -= get_num_nodes_in_tree(tree) * TOTAL_NODE_SIZE;
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
  /* Denotes the position in `arg_buf` where `tree` will be written. */
  binary_tree **const arg_buf_pos_tree = (void *)arg_buf;
  /* Denotes the position in `arg_buf` where `cur_node` will be written. */
  node_bt **const arg_buf_pos_cur_node =
      (void *)(arg_buf + sizeof(binary_tree *));
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

binary_tree *delete_node_from_tree(binary_tree *tree, node_bt *const target) {
  if (target != NULL) {
    if (target->parent != NULL) {
      /*
       * Overwrite the parent's pointer to `target` with the child nodes of
       * `target` to ensure the child nodes of `target` are not lost.
       */
      node_bt *remaining_branch;
      if (target->parent->left == target) {
        target->parent->left = target->left;
        remaining_branch = target->right;
      } else {
        target->parent->right = target->right;
        remaining_branch = target->left;
      }
      force_make_node_child_of(tree, target->parent, tree, remaining_branch);
    } else {
      /*
       * If the node has no parent, assume we are deleting the root node of
       * `tree`, so replace it with whichever child node is present (if both are
       * present, select the `left` and make `right` a child of `left`.).
       * Note that if neither are present, the value at `tree->root` will be
       * `NULL`.
       */
      if (tree->root->left != NULL) {
        tree->root->left = target->left;
        force_make_node_child_of(tree, tree->root->left, tree,
                                 tree->root->right);
      } else {
        tree->root = tree->root->right;
      }
    }
    tree->used_allocation -= sizeof(node_bt) + tree->value_size;
    /*
     * If the tree implements tracking of open blocks of memory, add the node
     * as an open block.
     */
    if (tree->unused_nodes != NULL) stack_push(tree->unused_nodes, target);
  }
  return tree;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
binary_tree *resize_tree(binary_tree *const tree, const size_t new_size) {
  binary_tree *const new_tree = realloc(tree, new_size);
  if (new_tree == NULL) return NULL;
  if (new_size < new_tree->used_allocation) {
    new_tree->used_allocation = new_size;
  }
  new_tree->allocation = new_size;

  return new_tree;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
binary_tree *resize_tree_s(binary_tree *tree, const size_t new_size) {
  if (new_size < tree->used_allocation) {
    const size_t NUM_NODES = get_num_nodes_in_tree(tree);
    const size_t NODES_AFFECTED =
        NUM_NODES - (new_size - sizeof(*tree)) / tree->value_size;
    const size_t NODE_SIZE = tree->value_size;
    for (size_t i = 0; i < NODES_AFFECTED; i++) {
      node_bt *cur_node =
          (void *)((byte *)tree->root + (NUM_NODES - i - 1) * NODE_SIZE);
      tree = delete_node_from_tree(tree, cur_node);
      if (tree == NULL) return NULL;
    }
  }

  binary_tree *new_tree = resize_tree(tree, new_size);
  if (new_tree == NULL) return NULL;

  return new_tree;
}

binary_tree *expand_binary_tree(binary_tree *const tree) {
  return resize_tree(tree, tree->allocation * BT_REALLOC_FACTOR);
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
  const node_bt *const node = (void *)((binary_tree *)tree_and_node + 1);
  return node->left == NULL || node->right == NULL;
}

/*
 * Helper function used by `find_open_descendant()`.
 *
 * \returns A pointer to a pointer to an open child node.
 */
static void *ret_node_with_open_child(void *const tree_and_node) {
  node_bt *const candidate = (void *)((binary_tree *)tree_and_node + 1);
  if (node_has_open_child(candidate)) return candidate;
  return NULL;
}

node_bt *find_open_descendant(node_bt *const origin) {
  return traverse_from(NULL, origin, ret_node_with_open_child,
                       node_has_open_child);
}

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

binary_tree *move_tree_node(binary_tree *dst_tree, binary_tree *src_tree,
                            node_bt **const src) {
  if (dst_tree == src_tree) return dst_tree;
  if (dst_tree->value_size != src_tree->value_size) return dst_tree;
  /* A tree is guaranteed to have leaf nodes, hence neither
   * `find_open_descendant()` nor `get_open_child_in_node()` can return
   * `NULL`.*/
  node_bt **const open_node =
      get_open_child_in_node(find_open_descendant(dst_tree->root));

  delete_node(src_tree, *src);
}

binary_tree *make_tree_node_child_of(binary_tree *dst_tree,
                                     node_bt *const dst_node,
                                     binary_tree *src_tree, node_bt *src_node) {
  if (dst_tree != src_tree) {
    src_node = remove_node_from_tree(src_tree, src_node);
    dst_tree = move_tree_node(dst_tree, src_tree, &src_node);
  }
  if (dst_node->left == NULL) {
    dst_node->left = src_node;
    src_node->parent = dst_node;
  } else if (dst_node->right == NULL) {
    dst_node->right = src_node;
    src_node->parent = dst_node;
  }
  return dst_tree;
}

binary_tree *force_make_tree_node_child_of(binary_tree *dst_tree,
                                           node_bt *dst_node,
                                           binary_tree *src_tree,
                                           node_bt *src_node) {
  dst_tree = make_node_child_of(dst_tree, dst_node, src_tree, src_node);
  if (src_node->parent == dst_node) return dst_tree;
  node_bt *open_candidate = find_open_descendant(src_node);
  /*
   * Move the child node (and its lineage) currently at `dst_node->left` to the
   * end of the lineage of `src_node`, thereby freeing up a child slot at
   * `dst_node` for `src_node`.
   */
  open_candidate = dst_node->left;
  dst_node->left = src_node;
}

binary_tree *add_freestanding_node(binary_tree *tree, node_bt **node) {
  const node_bt *node_actual = *node;
  if (node_actual == NULL) return tree;
  /* Use any unallocated nodes in `tree`. */
  node_bt *const open_node = get_unalloc_node(tree);
  if (open_node != NULL) {
    node_bt *const parent = find_open_descendant(tree->root);
    tree = make_node_child_of(tree, parent, tree, open_node);
    memcpy(open_node->value, node_actual->value, tree->value_size);
  }
}

node_bt *new_bt_node(const void *value, size_t value_size) {
  node_bt *const new_node = malloc(sizeof(node_bt) + value_size);
  if (new_node == NULL) return NULL;

  new_node->value = new_node + 1;
  new_node->parent = new_node->left = new_node->right = NULL;
  memcpy(new_node->value, value, value_size);
  return new_node;
}

#include <stdio.h>
static void *print_node(void *tree_and_node) {
  const node_bt *const node = (void *)((binary_tree *)tree_and_node + 1);
  static int bytes_written = 0;
  bytes_written = printf("%d ", *(int *)node->value);
  return &bytes_written;
}

int main(void) {
  const int data[] = {1, 2, 3};
  binary_tree *tree = new_binary_tree(data);
  node_bt *random_node = new_bt_node(data, sizeof(*data));
  delete_node_from_tree(tree, tree->root);
  traverse_from(tree, random_node, print_node, NULL);
  delete_tree(tree);

  return 0;
}
