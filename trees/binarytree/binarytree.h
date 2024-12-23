#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stddef.h>

#include "../trees.h"

typedef struct bt_node {
  void *value;
  struct bt_node *parent;
  struct bt_node *left, *right;
} bt_node;

/* Container structure for a binary tree data structure. */
typedef struct {
  bt_node *root;
  size_t num_nodes;
  size_t node_size;
  size_t allocation; /* Total bytes allocated for the tree and nodes. */
} binary_tree;

/*
 * This is a convenience macro for generating a `binary_tree` from an array.
 * Make sure the arguments have no side effects such as incrementation.
 */
#define new_binary_tree(data, length) \
  _new_binary_tree(data, sizeof *(data), length)

/*
 * Initializes a binary tree with the given elements from the passed array.
 * The first element in the array is always the root node, and subsequent
 * array elements fill into each node's `left` and `right` consecutively
 * starting from the left.
 *
 * Ex. Consider an array of seven elements: {7, 4, 9, 3, 0, 1, 0}.
 *     The resultant tree would look like this:
 *
 *         7
 *       /   \
 *      4     9
 *     / \   / \
 *    3   0 1   0
 */
binary_tree *_new_binary_tree(const void *data, size_t elem_size,
                              size_t length);

/*
 * Frees the passed binary tree's consumed memory and reassigns its pointer
 * to `NULL`.
 */
void delete_binary_tree(binary_tree **tree);

/*
 * Same as `delete_binary_tree()`, except this function will set all allocated
 * memory of the tree to zero, including pointers and tree/node statistics.
 */
void delete_binary_tree_s(binary_tree **tree);

/*
 * Adds the specified element as a child node of `parent_node`.
 * \return A pointer to the added node, or NULL upon failure.
 */
bt_node *add_binary_node(binary_tree *tree, const void *elem);

/*
 * Removes `target` from `src`, thereby causing `target` to have no parent
 * (assumed `NULL`).
 *
 * \return The removed node.
 */
bt_node *remove_binary_node(binary_tree *src, bt_node *target);

/*
 * This function makes `child` a child node of `parent`, including any child
 * nodes.
 */
void reparent_binary_node(const bt_node *parent, const bt_node *child);
#endif
