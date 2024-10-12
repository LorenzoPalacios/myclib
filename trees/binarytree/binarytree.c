#include "binarytree.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vadefs.h>

#include "../trees.h"

#define BT_REALLOC_FACTOR (2)

binary_tree *_new_binary_tree(const void *const data, const size_t elem_size,
                              const size_t length) {
  const size_t NODE_SIZE = sizeof(bt_node) + elem_size;
  const size_t REQUIRED_MEM = length * NODE_SIZE + sizeof(binary_tree);
  binary_tree *const tree_obj = malloc(REQUIRED_MEM);
  /* Increment past the tree header. */
  bt_node *const nodes_mem = (void *)(tree_obj + 1);
  if (tree_obj == NULL) return NULL;

  tree_obj->num_nodes = length;
  tree_obj->node_size = NODE_SIZE;
  tree_obj->used_allocation = tree_obj->allocation = REQUIRED_MEM;
  tree_obj->open_nodes = NULL;

  for (size_t i = 0; i < length; i++) {
    bt_node *const cur_node = (void *)((char *)nodes_mem + i * NODE_SIZE);
    bt_node *parent_node;
    if (i == 0)
      parent_node = NULL;
    else {
      parent_node = (void *)((char *)cur_node - NODE_SIZE * (i / 2 + 1));
      if (i & 1)
        parent_node->left = cur_node;
      else
        parent_node->right = cur_node;
    }
    void *const cur_node_data = (char *)cur_node + sizeof(bt_node);
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

bt_node *remove_node_from_tree(binary_tree *const tree, bt_node *const target) {
  bt_node *const node_copy = malloc(tree->node_size);
  if (node_copy == NULL) return NULL;
  memcpy(node_copy, target, tree->node_size);
  delete_node_from_tree_s(tree, target);
  node_copy->parent = NULL;
  return node_copy;
}

void operate_over_lineage(bt_node *const origin,
                          void (*const op)(bt_node *node, va_list *args),
                          va_list *const args) {
  bt_node *cur_node = origin->left;
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

size_t get_depth(bt_node *const origin) {
  size_t left_branch_depth = 0;
  for (bt_node *cur_node = origin->left; cur_node != NULL;
       left_branch_depth++) {
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }

  size_t right_branch_depth = 0;
  for (bt_node *cur_node = origin->right; cur_node != NULL;
       right_branch_depth++) {
    if (cur_node->right == NULL)
      cur_node = cur_node->left;
    else
      cur_node = cur_node->right;
  }
  return left_branch_depth > right_branch_depth ? left_branch_depth
                                                : right_branch_depth;
}

size_t count_descendant_nodes(bt_node *const origin) {
  size_t count = 0;
  for (bt_node *cur_node = origin->left; cur_node != NULL; count++) {
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }

  for (bt_node *cur_node = origin->right; cur_node != NULL; count++) {
    if (cur_node->left == NULL)
      cur_node = cur_node->right;
    else
      cur_node = cur_node->left;
  }
  return count;
}

/* NEEDS REDESIGN/REWRITE */
void delete_node_and_lineage(binary_tree *const tree, bt_node *target) {
  {
    bt_node *parent = target->parent;
    if (parent->left == target)
      parent->left = NULL;
    else
      parent->right = NULL;
  }
  const size_t REMOVED_NODES = get_depth(target) + 1;
  tree->num_nodes -= REMOVED_NODES;
  tree->used_allocation -= REMOVED_NODES * tree->node_size;
}

/*
 * Searches upwards along the ancestry of `origin` until a node is found whose
 * `left` and `right` pointers are not `NULL`. Such a node is considered
 * divergent since, during tree traversal, a search algorithm must choose
 * either the `left` or `right` branch of that node.
 *
 * \return A pointer to the first ancestral node of `origin` containing a branch
 * divergence or `NULL` if no suitable node is found.
 */
bt_node *next_ancestral_divergence(const bt_node *origin) {
  while (origin->parent != NULL) {
    bt_node *const parent = origin->parent;
    if (parent->left != NULL && parent->right != NULL) return parent;
    origin = parent;
  }
  return NULL;
}

bt_node **search_for_node(bt_node *const origin, const bt_node *const target) {
  /*
   * The function will search along the left branch of `origin` and will deviate
   * to a right branch if and only if the `left` pointer of a traversed node is
   * `NULL`. If both `left` and `right` are `NULL`, then we backtrack to this
   * node which will have a right branch to be traversed.
   *
   * Note that this node will have already been traversed prior to backtracking,
   * hence why we can always select the right branch of this node.
   */
  bt_node *last_divergence = NULL;
  bt_node *cur_node = origin;
  bt_node *next_node = NULL;
  while (cur_node != NULL) {
    if (cur_node->left == target) return &cur_node->left;
    if (cur_node->right == target) return &cur_node->right;
    printf("%zu\n", *(size_t *)cur_node->value);
    if (cur_node->left != NULL) {
      /*
       * If both `left` and `right` are valid, continue down the left branch and
       * save `cur_node` as the last divergent node.
       */
      if (cur_node->right != NULL) last_divergence = cur_node;
      next_node = cur_node->left;
    } else if (cur_node->right != NULL) {
      next_node = cur_node->right;
    } else {
      /* If there is no divergence along the traversed path, we're done. */
      if (last_divergence == NULL) break;
      next_node = last_divergence->right;
      last_divergence = next_ancestral_divergence(last_divergence);
    }
    cur_node = next_node;
  }
  return NULL;
}

binary_tree *delete_node_from_tree_s(binary_tree *tree, bt_node *const target) {
  if (target != NULL) {
    bt_node *const parent = target->parent;
    if (parent != NULL) {
      /*
       * Overwrite the parent's pointer to `target` with the child nodes of
       * `target` to ensure the lineage of `target` is not lost.
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
      /* If the node has no parent, assume it is the root node. */
      tree->root = target->left == NULL ? target->right : target->left;
    }
    tree->used_allocation -= tree->node_size;
    /*
     * If the tree implements tracking of open blocks of memory, add the node
     * as an open block.
     */
    if (tree->open_nodes != NULL) {
      tree = add_open_node(tree, target);
      if (tree == NULL) return NULL;
    }
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
      bt_node *cur_node =
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

binary_tree *add_open_node(binary_tree *tree, bt_node *const open_node) {
  if (tree->open_nodes == NULL) return NULL;
  const size_t ALLOCATION = tree->allocation;
  const size_t USED_ALLOCATION = tree->used_allocation;
  if (ALLOCATION - USED_ALLOCATION < sizeof(bt_node *)) {
    tree = expand_tree(tree);
    if (tree == NULL) return NULL;
  }
  /*
   * This direct access of `tree->open_nodes` exists for two reasons:
   * 1. This gives the address of the terminating `NULL` pointer in
   * `tree->open_node`.
   * 2. This will not modify `tree->open_nodes` and will therefore not interfere
   * with any code reliant upon `tree->open_nodes`, such as `get_open_node`.
   */
  const bt_node **stack_terminator =
      (void *)((char *)tree + tree->used_allocation - sizeof(tree->open_nodes));
  *stack_terminator = open_node;
  *(stack_terminator + 1) = NULL;
  tree->used_allocation += sizeof(bt_node *);
  return tree;
}

binary_tree *init_open_nodes(binary_tree *tree) {
  const size_t ALLOCATION = tree->allocation;
  const size_t USED_ALLOCATION = tree->used_allocation;
  if (ALLOCATION - USED_ALLOCATION < sizeof(bt_node *)) {
    tree = expand_tree(tree);
    if (tree == NULL) return NULL;
  }
  tree->open_nodes = (void *)((char *)tree + USED_ALLOCATION);
  *tree->open_nodes = NULL;
  tree->used_allocation += sizeof(tree->open_nodes);
  return tree;
}

/* write documentation */
bt_node *get_open_node(binary_tree *const tree) {
  if (tree->open_nodes == NULL) return NULL;
  return *(tree->open_nodes++);
}

bt_node **find_open_descendant(bt_node *const origin) {
  return search_for_node(origin, NULL);
}

/* NEEDS REDESIGN/REWRITE */
void make_node_child_of(bt_node *const src, bt_node *const dst) {
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
void force_make_node_child_of(bt_node *const src, bt_node *const dst) {
  make_node_child_of(src, dst);
  if (src->parent == dst) return;
  bt_node **open_candidate = find_open_descendant(src);
  *open_candidate = dst->left;
  dst->left = src;
}

int main(void) {
  const size_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
  binary_tree *a = new_binary_tree(data, sizeof(data) / sizeof(*data));
  bt_node **found_node = search_for_node(a->root, a->root);
  printf("addressof: %td\n", (ptrdiff_t)found_node);
  if (found_node != NULL) {
    printf("node pointer: %td\n", (ptrdiff_t)*found_node);
    printf("pointer dist from root: %td\n",
           (char *)*found_node - (char *)a->root);
    if ((*found_node)->parent != NULL)
      printf("node value: %zu\n", *(size_t *)(*found_node)->parent->value);
  }
  return 0;
}
