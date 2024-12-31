#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Stacks are used to track unallocated nodes in trees and in
// `bt_traverse()` to avoid the usage of recursion.
#include "../../stack/stack.h"

#define NULL_INDEX (SIZE_MAX)

typedef struct bt_node bt_node;

// `bt_node` stands for node_binary_tree.
typedef struct bt_node {
  size_t parent_index;
  size_t left_index;
  size_t right_index;
} bt_node;

// Binary tree header.
typedef struct {
  stack *unused_nodes;  // Contains pointers to unused nodes.
  size_t allocation;    // Total bytes allocated for the tree and nodes.
  size_t root_index;
  size_t value_size;  // Size (in bytes) of a stored value.
} binary_tree;

// This is a convenience macro for generating a binary tree from an array.
// Use caution if the arguments to this macro have side effects.
#define bt_new(arr) bt_new_(arr, sizeof *(arr), sizeof(arr) / sizeof *(arr))

#define bt_delete(tree) bt_delete_(&(tree))

#define bt_delete_s(tree) bt_delete_s_(&(tree))

/*
 * Initializes a binary tree with the given elements from the passed array.
 * The first element in the array is always the root node and subsequent
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
 *
 * Shown linearly:
 * `()` - Denotes a node.
 * `{}` - Denotes a group of nodes.
 * `->` - Signifies that the left-hand node is the parent of the right-hand node
 *        or group.
 * `,`  - Separator for nodes in a group.
 *
 * `(7) -> {(4) -> {(3), (0)}}`,
 * `(7) -> {(9) -> {(1), (0)}}`
 *
 * \return A balanced binary tree whose structure is in accordance with the
 * above example or `NULL` if the tree could not be created.
 */
binary_tree *bt_new_(const void *data, size_t elem_size, size_t num_elems);

/*
 * Same as `bt_delete_()`, except this function will write `0` across
 * the allocated memory of `tree`.
 */
void bt_delete_(binary_tree **tree);

/*
 * Same as `bt_delete_()`, except this function will set all allocated
 * memory of the tree to zero, including pointers and tree/node statistics.
 */
void bt_delete_s_(binary_tree **tree);

binary_tree *bt_expand(binary_tree *tree);

bt_node *get_unalloc_node(binary_tree *tree);

/*
 * Registers `open_node` as an open block of memory in `tree` for any new
 * incoming nodes to fill.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 */
void register_unalloc_node(binary_tree *tree, bt_node *open_node);

/*
 * Resizes the memory allocated for `tree` to `new_size`.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note If `new_size` is less than `tree->used_allocation`, then tree data
 * can be corrupted. If this can occur, consider using `resize_tree_s()`.
 */
binary_tree *bt_resize(binary_tree *tree, size_t new_capacity);

/*
 * Traverses all of the descendant nodes of `origin`, including `origin` itself,
 * and passes `tree` and each traversed node as a pointer to `operation` until
 * `operation` returns `true` or all the nodes in the lineage of `origin` have
 * been traversed.
 *
 * This function is NOT recursive; it has a dependency on the `stack` data
 * structure implemented elsewhere in this library.
 *
 * \note
 * - If a traversed node meets the criteria for `stop_condition` to return
 * `true`, `operation` will still be run for that node.
 *
 * - If the internal stack used for traversal does not use most of its
 * capacity after a few calls to this function (internal library calls count
 * towards this), it will shrink itself to reduce memory footprint.
 */
void bt_traverse(binary_tree *tree, bt_node *from,
                 bool (*operation)(binary_tree *tree, bt_node *node));

#endif
