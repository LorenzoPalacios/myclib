#include "binarytree.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vadefs.h>

#include "../../stack/stack.h"
#include "../trees.h"

#define BT_REALLOC_FACTOR (2)

binary_tree *_new_binary_tree(const void *const data, const size_t elem_size,
                              const size_t length) {
  const size_t NODE_SIZE = sizeof(node_bt) + elem_size;
  const size_t REQUIRED_MEM = length * NODE_SIZE + sizeof(binary_tree);
  binary_tree *const tree_obj = malloc(REQUIRED_MEM);
  /* Increment past the tree header. */
  node_bt *const nodes_mem = (void *)(tree_obj + 1);
  if (tree_obj == NULL) return NULL;

  tree_obj->num_nodes = length;
  tree_obj->node_size = NODE_SIZE;
  tree_obj->used_allocation = tree_obj->allocation = REQUIRED_MEM;
  /* The tracking of unallocated nodes is an opt-in feature, hence why its
   * initial value is `NULL`. */
  tree_obj->unallocated_nodes = NULL;

  for (size_t i = 0; i < length; i++) {
    node_bt *const cur_node = (void *)((char *)nodes_mem + i * NODE_SIZE);
    node_bt *parent_node;
    if (i == 0)
      parent_node = NULL;
    else {
      parent_node = (void *)((char *)cur_node - NODE_SIZE * (i / 2 + 1));
      if (i & 1)
        parent_node->left = cur_node;
      else
        parent_node->right = cur_node;
    }
    void *const cur_node_data = (char *)cur_node + sizeof(node_bt);
    memcpy(cur_node_data, (char *)data + i * elem_size, elem_size);
    cur_node->value = cur_node_data;
    cur_node->parent = parent_node;
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

node_bt *remove_node_from_tree(binary_tree *const tree, node_bt *const target) {
  node_bt *const node_copy = malloc(tree->node_size);
  if (node_copy == NULL) return NULL;
  memcpy(node_copy, target, tree->node_size);
  delete_node_from_tree_s(tree, target);
  node_copy->parent = NULL;
  return node_copy;
}

void operate_over_lineage(node_bt *const origin,
                          void (*const op)(node_bt *node, va_list *args),
                          va_list *const args) {
  node_bt *cur_node = origin->left;
  while (cur_node != NULL) {
    op(cur_node, args);
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }

  cur_node = origin->right;
  while (cur_node != NULL) {
    op(cur_node, args);
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }
}

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

size_t count_descendant_nodes(node_bt *const origin) {
  size_t count = 0;
  for (node_bt *cur_node = origin->left; cur_node != NULL; count++) {
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }

  for (node_bt *cur_node = origin->right; cur_node != NULL; count++) {
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }
  return count;
}

/* NEEDS REDESIGN/REWRITE */
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
  tree->used_allocation -= DELETED_NODES * tree->node_size;
}

/*
 * Searches along the ancestry of `origin` until a node is found whose `left`
 * and `right` pointers are not `NULL`. Such a node is considered divergent
 * since, during tree traversal, a search algorithm must choose either the
 * `left` or `right` branch of that node.
 *
 * \return A pointer to the first ancestral node of `origin` containing a branch
 * divergence or `NULL` if no suitable node is found.
 */
node_bt *next_ancestral_divergence(const node_bt *const origin) {
  node_bt *cur_node = origin->parent;
  while (cur_node != NULL) {
    if (cur_node->left != NULL && cur_node->right != NULL) return cur_node;
    cur_node = cur_node->parent;
  }
  return NULL;
}

node_bt **traverse_from(node_bt *const origin, const node_bt *const target) {
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
   * Recursion is the natural solution to tree traversal, however it brings the
   * issue of stack overflow, which this implementation seeks to avoid.
   */
  static stack *divergent_nodes = NULL;
  /*
   * If the stack is uninitialized, initialize it.
   * Otherwise, ensure the stack does not contain any divergent nodes from prior
   * searches by clearing its contents.
   */
  if (divergent_nodes == NULL)
    divergent_nodes = new_empty_stack(12, sizeof(node_bt *));
  else
    clear_stack(divergent_nodes);

  node_bt *cur_node = origin;
  node_bt *next_node = NULL;
  size_t iter = 0;
  while (cur_node != NULL) {
    iter++;
    if (cur_node->left == target) return &cur_node->left;
    if (cur_node->right == target) return &cur_node->right;
    if (cur_node->left != NULL) {
      /*
       * If both `left` and `right` are valid, continue down the left branch and
       * save `cur_node` as a divergent node.
       *
       * Taking the address of `cur_node` is necessary since `stack_push()`
       * expects the `elem` argument to be a pointer to data. Since `cur_node`
       * is a pointer to a node, but we need to keep track of that pointer, we
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
    cur_node = next_node;
  }
  printf("iterations: %zu\n", iter);
  return NULL;
}

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
       * neither are present, the value at `tree->root` will be NULL.
       */
      tree->root = (target->left != NULL ? target->left : target->right);
    }
    tree->used_allocation -= tree->node_size;
    /*
     * If the tree implements tracking of open blocks of memory, add the node
     * as an open block.
     */
    if (tree->unallocated_nodes != NULL)
      stack_push(tree->unallocated_nodes, target);
  }
  return tree;
}

binary_tree *resize_tree(binary_tree *const tree, const size_t new_size) {
  binary_tree *const new_tree = realloc(tree, new_size);
  if (new_tree == NULL) return NULL;
  if (new_size < new_tree->used_allocation) {
    new_tree->used_allocation = new_size;
    new_tree->num_nodes = (new_size - sizeof(*new_tree)) / new_tree->node_size;
  }
  new_tree->allocation = new_size;

  return new_tree;
}

binary_tree *resize_tree_s(binary_tree *tree, const size_t new_size) {
  if (new_size < tree->used_allocation) {
    const size_t NODES_AFFECTED =
        tree->num_nodes - (new_size - sizeof(*tree)) / tree->node_size;
    const size_t NODE_SIZE = tree->node_size;
    const size_t NUM_NODES = tree->num_nodes;
    for (size_t i = 0; i < NODES_AFFECTED; i++) {
      node_bt *cur_node =
          (void *)((char *)tree->root + (NUM_NODES - i - 1) * NODE_SIZE);
      tree = delete_node_from_tree_s(tree, cur_node);
      if (tree == NULL) return NULL;
    }
  }

  binary_tree *new_tree = resize_tree(tree, new_size);
  if (new_tree == NULL) return NULL;

  return new_tree;
}

binary_tree *expand_tree(binary_tree *const tree) {
  return resize_tree(tree, tree->allocation * BT_REALLOC_FACTOR);
}

binary_tree *add_open_node(binary_tree *tree, node_bt *const open_node) {
  if (tree->unallocated_nodes == NULL) return NULL;
  const size_t ALLOCATION = tree->allocation;
  const size_t USED_ALLOCATION = tree->used_allocation;
  if (ALLOCATION - USED_ALLOCATION < sizeof(node_bt *)) {
    tree = expand_tree(tree);
    if (tree == NULL) return NULL;
  }
  /*
   * This direct access of `tree->unallocated_nodes` exists for two reasons:
   * 1. This gives the address of the terminating `NULL` pointer in
   * `tree->open_node`.
   * 2. This will not modify `tree->unallocated_nodes` and will therefore not
   * interfere with any code reliant upon `tree->unallocated_nodes`, such as
   * `get_open_node`.
   */
  const node_bt **stack_terminator =
      (void *)((char *)tree + tree->used_allocation -
               sizeof(tree->unallocated_nodes));
  *stack_terminator = open_node;
  *(stack_terminator + 1) = NULL;
  tree->used_allocation += sizeof(node_bt *);
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
    tree = expand_tree(tree);
    if (tree == NULL) return NULL;
  }
  tree->unallocated_nodes = (void *)((char *)tree + tree->used_allocation);

  *(tree->unallocated_nodes) =
      (stack){/*
               * Set the pointer to the stack's data to the
               * memory just after the stack header.
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

node_bt **find_open_descendant(node_bt *const origin) {
  return search_for_node(origin, NULL);
}

/* NEEDS REDESIGN/REWRITE */
void make_node_child_of(node_bt *const src, node_bt *const dst) {
  if (dst->left == NULL) {
    dst->left = src;
    return;
  }
  if (dst->right == NULL) {
    dst->right = src;
    return;
  }
}

/* NEEDS REDESIGN/REWRITE */
void force_make_node_child_of(node_bt *const src, node_bt *const dst) {
  make_node_child_of(src, dst);
  if (src->parent == dst) return;
  node_bt **open_candidate = find_open_descendant(src);
  *open_candidate = dst->left;
  dst->left = src;
}

#include <time.h>

int main(void) {
  size_t data[0x10000];
  srand(time(NULL));
  for (size_t i = 0; i < sizeof(data) / sizeof(*data); i++) data[i] = rand();
  binary_tree *tree = new_binary_tree(data);


  return 0;
}
