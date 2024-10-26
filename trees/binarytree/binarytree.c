#include "binarytree.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../stack/stack.h"
#include "../trees.h"

typedef unsigned char byte;

/* Reallocation factor used by `expand_binary_tree()`. */
#define BT_REALLOC_FACTOR (2)

binary_tree *_new_binary_tree(const void *const data, const size_t elem_size,
                              const size_t length) {
  const size_t NODE_SIZE = sizeof(node_bt) + elem_size;
  const size_t TOTAL_REQ_MEM = length * NODE_SIZE + sizeof(binary_tree);
  binary_tree *const tree_obj = malloc(TOTAL_REQ_MEM);
  /* The nodes will be stored directly after the tree header. */
  node_bt *const nodes_mem = (void *)(tree_obj + 1);
  /*
   * The values will be stored directly after the nodes.
   * Originally, the value for each node was stored directly after their
   * container node. This is done to avoid misalignment.
   */
  byte *const values_mem = (byte *)(nodes_mem + length);
  if (tree_obj == NULL) return NULL;

  tree_obj->num_nodes = length;
  tree_obj->value_size = elem_size;
  tree_obj->used_allocation = tree_obj->allocation = TOTAL_REQ_MEM;
  /* The tracking of unallocated nodes is an opt-in feature, hence why its
   * initial value is `NULL`. */
  tree_obj->unallocated_nodes = NULL;

  for (size_t i = 0; i < length; i++) {
    node_bt *const cur_node = nodes_mem + i;
    byte *const cur_node_data = values_mem + i * elem_size;
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
    memcpy(cur_node_data, (byte *)data + i * elem_size, elem_size);
    cur_node->value = cur_node_data;
    cur_node->parent = parent_node;
    /* Each new node will have no children from the outset. */
    cur_node->left = cur_node->right = NULL;
  }
  tree_obj->root = nodes_mem; /* The first node is always the root node. */
  return tree_obj;
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
  delete_node_from_tree_s(tree, target);
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
 * \return A pointer to a `size_t` which describes the
 * number of nodes traversed.
 */
static void *count_node(node_bt *const node) {
  static size_t count = 0;
  count++;
  /*
   * `traverse_from()` will not pass a `NULL` pointer to this function,
   * so we can pass `NULL` from `count_descendant_nodes()` to tell this function
   * to reset `count`.
   */
  if (node == NULL) count = 0;
  return &count;
}

size_t count_descendant_nodes(node_bt *const origin) {
  /*
   * Since `traverse_from()` returns the last returned value of `op`,
   * which is `count_node()`, it will return a pointer to the number of
   * traversed nodes. Then, we subtract one, since `traverse_from()` will count
   * `origin` when we only need to count the descendants of `origin`.
   */
  const size_t count = *(size_t *)traverse_from(origin, count_node, NULL) - 1;
  count_node(NULL); /* Tells `count_node()` to reset its counter. */
  return count;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
void delete_node_and_lineage(binary_tree *const tree, node_bt *target) {
  {
    node_bt *parent = target->parent;
    if (parent->left == target)
      parent->left = NULL;
    else
      parent->right = NULL;
  }
  /* Adding one to account for the deletion of `target. */
  const size_t DELETED_NODES = get_depth(target) + 1;
  tree->num_nodes -= DELETED_NODES;
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

void *traverse_from(node_bt *const origin, void *(*const op)(node_bt *),
                    bool (*const stop_condition)(node_bt *)) {
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

  bool low_stack_usage = true;
  void *ret_val = NULL;
  node_bt *cur_node = origin;
  node_bt *next_node = NULL;
  while (cur_node != NULL) {
    if (op != NULL) ret_val = op(cur_node);
    if (stop_condition != NULL)
      if (stop_condition(cur_node)) break;
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
binary_tree *delete_node_from_tree_s(binary_tree *tree, node_bt *const target) {
  if (target != NULL) {
    node_bt *const parent = target->parent;
    if (parent != NULL) {
      /*
       * Overwrite the parent's pointer to `target` with the child nodes of
       * `target` to ensure the child nodes of `target` are not lost.
       */
      if (parent->left == target) {
        parent->left = target->left;
        if (target->right != NULL)
          *find_open_descendant(parent->right) = target->right;
      } else {
        parent->right = target->right;
        if (target->left != NULL)
          *find_open_descendant(parent->left) = target->left;
      }
    } else {
      /*
       * If the node has no parent, assume we are deleting the root node and
       * replace its position with whichever child node is present. Note that if
       * neither are present, the value at `tree->root` will be `NULL`.
       */
      tree->root = (target->left != NULL ? target->left : target->right);
    }
    tree->used_allocation -= tree->value_size;
    /*
     * If the tree implements tracking of open blocks of memory, add the node
     * as an open block.
     */
    if (tree->unallocated_nodes != NULL)
      stack_push(tree->unallocated_nodes, target);
  }
  return tree;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
binary_tree *resize_tree(binary_tree *const tree, const size_t new_size) {
  binary_tree *const new_tree = realloc(tree, new_size);
  if (new_tree == NULL) return NULL;
  if (new_size < new_tree->used_allocation) {
    new_tree->used_allocation = new_size;
    new_tree->num_nodes = (new_size - sizeof(*new_tree)) / new_tree->value_size;
  }
  new_tree->allocation = new_size;

  return new_tree;
}

/* NEEDS REWRITE FOR COMPLIANCE WITH NEW TREE MEMORY STRUCTURE */
binary_tree *resize_tree_s(binary_tree *tree, const size_t new_size) {
  if (new_size < tree->used_allocation) {
    const size_t NODES_AFFECTED =
        tree->num_nodes - (new_size - sizeof(*tree)) / tree->value_size;
    const size_t NODE_SIZE = tree->value_size;
    const size_t NUM_NODES = tree->num_nodes;
    for (size_t i = 0; i < NODES_AFFECTED; i++) {
      node_bt *cur_node =
          (void *)((byte *)tree->root + (NUM_NODES - i - 1) * NODE_SIZE);
      tree = delete_node_from_tree_s(tree, cur_node);
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

binary_tree *push_unalloc_node(binary_tree *tree, node_bt *const open_node) {
  stack *const unalloc_nodes_stk = tree->unallocated_nodes;
  if (unalloc_nodes_stk == NULL) return NULL;
  const size_t STK_AVAILABLE_ALLOC = unalloc_nodes_stk - unalloc_nodes_stk;
  if (STK_AVAILABLE_ALLOC < sizeof(node_bt **)) {
    const size_t REQ_ALLOC = STK_AVAILABLE_ALLOC + sizeof(node_bt **);
    const size_t TREE_AVAILABLE_ALLOC =
        tree->allocation - tree->used_allocation;
    if (TREE_AVAILABLE_ALLOC < REQ_ALLOC) {
      tree = expand_binary_tree(tree);
      if (tree == NULL) return NULL;
    }
    unalloc_nodes_stk->capacity = REQ_ALLOC;
  }
  /*
   * Using a heapless function guarantees that pushing a new element onto the
   * stack will not incur modifications to the allocation of memory for
   * `tree`.
   */
  heapless_stack_push(unalloc_nodes_stk, &open_node);
  return tree;
}

binary_tree *init_open_nodes(binary_tree *tree) {
  const size_t AVAILABLE_MEM = tree->allocation - tree->used_allocation;
  /*
   * Allocate enough memory for a stack that can hold one pointer to an
   * unallocated node.
   */
  const size_t REQ_ALLOCATION = sizeof(stack) + sizeof(node_bt **);
  if (AVAILABLE_MEM < REQ_ALLOCATION) {
    tree = expand_binary_tree(tree);
    if (tree == NULL) return NULL;
  }
  tree->unallocated_nodes = (void *)((byte *)tree + tree->used_allocation);

  *(tree->unallocated_nodes) = (stack){
      /*
       * Sets the data pointer to the memory just after the stack header.
       */
      .data = tree->unallocated_nodes + 1,
      .capacity = REQ_ALLOCATION,
      .used_capacity = 0,
      .elem_size = sizeof(node_bt **),
      .length = 0};

  tree->used_allocation += REQ_ALLOCATION;
  return tree;
}

/* write documentation */
node_bt *get_open_node(binary_tree *const tree) {
  if (tree->unallocated_nodes == NULL) return NULL;
  return stack_pop(tree->unallocated_nodes);
}

/* Helper function used by `find_open_descendant()`. */
static bool node_has_open_child(node_bt *const candidate) {
  return candidate->left == NULL || candidate->right == NULL;
}

/*
 * Helper function used by `find_open_descendant()`.
 *
 * \returns A pointer to a pointer to an open child node.
 */
static void *ret_open_child(node_bt *const candidate) {
  if (candidate->left == NULL) return &candidate->left;
  if (candidate->right == NULL) return &candidate->right;
  return NULL;
}

node_bt **find_open_descendant(node_bt *const origin) {
  return traverse_from(origin, ret_open_child, node_has_open_child);
}

bool nodes_exist_in_same_tree(const node_bt *const node_1,
                              const node_bt *const node_2) {
  const node_bt *node_1_root_node = NULL;
  for (const node_bt *cur_node = node_1->parent; cur_node->parent != NULL;
       cur_node = cur_node->parent) {
    node_1_root_node = cur_node;
  }
  const node_bt *node_2_root_node = NULL;
  for (const node_bt *cur_node = node_2->parent; cur_node->parent != NULL;
       cur_node = cur_node->parent) {
    node_2_root_node = cur_node;
  }
  return node_1_root_node == node_2_root_node;
}

binary_tree *make_node_child_of(binary_tree *dst_tree, node_bt *const dst,
                                binary_tree *src_tree, node_bt *src) {
  if (dst_tree != src_tree) {
    src = remove_node_from_tree(src_tree, src);
    // dst_tree = add_node_to_bt(dst_tree, src);
    src_tree->num_nodes--;
    dst_tree->num_nodes++;
  }
  if (dst->left == NULL) {
    dst->left = src;
    src->parent = dst;
  } else if (dst->right == NULL) {
    dst->right = src;
    src->parent = dst;
  }
  return dst_tree;
}

void force_make_node_child_of(binary_tree *dst_tree, node_bt *const dst,
                              binary_tree *src_tree, node_bt *const src) {
  make_node_child_of(dst_tree, dst, src_tree, src);
  if (src->parent == dst) return;
  node_bt **const open_candidate = find_open_descendant(src);
  /*
   * Move the child node (and its lineage) currently at `dst->left` to the end
   * of the lineage of `src`, thereby freeing up a child slot at `dst` for
   * `src`.
   */
  *open_candidate = dst->left;
  dst->left = src;
}

#include <stdio.h>
