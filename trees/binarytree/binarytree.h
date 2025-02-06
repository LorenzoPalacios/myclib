#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Stacks are used to track unallocated nodes in trees and in
// `bt_traverse()` to avoid the usage of recursion.
#include "../../stack/stack.h"

#define NULL_INDEX (SIZE_MAX)

// Binary tree node.
typedef struct bt_node bt_node;

// Binary tree header.
typedef struct binary_tree binary_tree;

/**
 * @brief Macro to create a new binary tree.
 *
 * This macro initializes a new binary tree with the given data.
 *
 * @param data The data to initialize the binary tree with.
 */
#define bt_new(data) bt_new_(data, sizeof *(data), sizeof(data) / sizeof *(data))

/**
 * @brief Macro to traverse a binary tree.
 *
 * This macro traverses the binary tree starting from the root node and applies the given operation.
 *
 * @param tree The binary tree to traverse.
 * @param op The operation to apply to each node during traversal.
 */
#define bt_traverse_tree(tree, op) bt_traverse(tree, get_root_node(tree), op)

/**
 * @brief Adds a new node with the given value to the binary tree.
 *
 * @param tree A pointer to the binary tree.
 * @param value A pointer to the value to be added.
 * @return A pointer to the updated binary tree, or `NULL` if the operation
 * fails.
 * @note The added value must be the same size as `tree->value_size`.
 */
binary_tree *bt_add_node(binary_tree *tree, const void *value);

/**
 * @brief Deletes the binary tree and frees its memory.
 *
 * @param tree A pointer to the binary tree to be deleted.
 */
void bt_delete(binary_tree *tree);

/**
 * @brief Deletes a specific node and its descendants from the binary tree.
 *
 * @param tree A pointer to the binary tree.
 * @param node A pointer to the node to be deleted.
 */
void bt_delete_node(binary_tree *tree, bt_node *node);

/**
 * @brief Expands the binary tree to accommodate more nodes.
 *
 * @param tree A pointer to the binary tree.
 * @return A pointer to the expanded binary tree, or NULL if the operation
 * fails.
 */
binary_tree *bt_expand(binary_tree *tree);

/**
 * @brief Initializes a binary tree with a specified number of nodes.
 *
 * @param value_size The size of each value in bytes.
 * @param node_cnt The number of nodes to allocate.
 * @return A pointer to the initialized binary tree, or NULL if the operation
 * fails.
 */
binary_tree *bt_init(size_t value_size, size_t node_cnt);

/**
 * @brief Initializes a binary tree with the given elements from the passed
 * array.
 *
 * The first element in the array is always the root node and subsequent
 * array elements fill into each node's `left` and `right` consecutively
 * starting from the left.
 *
 * Ex. Consider an array of seven elements: {7, 4, 9, 3, 0, 1, 0}.
 *     The resultant binary tree would look like this:
 *
 *         7
 *       /   \
 *      4     9
 *     / \   / \
 *    3   0 1   0
 *
 * @param data A pointer to the array of values.
 * @param value_size The size of each value in bytes.
 * @param num_values The number of values in the array.
 * @return A pointer to the initialized binary tree, or NULL if the operation
 * fails.
 */
binary_tree *bt_new_(const void *data, size_t value_size, size_t num_values);

/**
 * @brief Resizes the memory allocated for the binary tree to the new capacity.
 *
 * @param tree A pointer to the binary tree.
 * @param new_capacity The new capacity for the binary tree.
 * @return A pointer to the resized binary tree, or NULL if the operation fails.
 * @note If `new_capacity` would cause active nodes to be lost, the operation
 * will fail.
 */
binary_tree *bt_resize(binary_tree *tree, size_t new_capacity);

/**
 * @brief Swaps the values of two nodes in a binary tree.
 *
 * This function takes a binary tree and two nodes within that tree, and swaps
 * their values. The structure of the tree remains unchanged.
 *
 * @param tree A pointer to the binary tree.
 * @param node_1 A pointer to the first node whose value is to be swapped.
 * @param node_2 A pointer to the second node whose value is to be swapped.
 */
void bt_swap_values(binary_tree *tree, bt_node *node_1, bt_node *node_2);

/**
 * @brief Traverses all of the descendant nodes of `origin` and `origin`.
 *
 * This function passes `tree` and each traversed node to `operation` until
 * `operation` returns `false` or all the nodes in the lineage of `origin`
 * have been traversed.
 *
 * This function is NOT recursive; it has a dependency on the `stack` data
 * structure implemented elsewhere in this library.
 *
 * @param tree A pointer to the binary tree.
 * @param from A pointer to the starting node for traversal.
 * @param operation A function pointer to the operation to be performed on each
 * node.
 */
void bt_traverse(binary_tree *tree, bt_node *from,
                 bool (*operation)(binary_tree *tree, bt_node *node));

#endif
