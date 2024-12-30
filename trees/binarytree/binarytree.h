#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Stacks are used to track unallocated nodes in trees and in
// `bt_traverse()` to avoid the usage of recursion.
#include "../../stack/stack.h"

#define NULL_INDEX (SIZE_MAX)

/*
 * Determines how many calls to `bt_traverse()` (where the internal
 * stack's `used_capacity` never exceeds the product of its `capacity` and
 * `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT`) must be made before shrinking the
 * internal stack's allocated memory.
 */
#define TRAVERSAL_STACK_SHRINK_COUNTER_MAX (5)

/*
 * If `bt_traverse` is called `TRAVERSAL_STACK_SHRINK_COUNTER` times
 * and the `used_capacity` of the internal stack used by
 * `bt_traverse()` is less than the product of this multiplier and the
 * stack's `capacity`, the internal stack will shrink its allocated memory.
 *
 * Expressed mathematically:
 *
 * c - The capacity of the stack.
 * u - The used capacity of the stack.
 * k - The value of `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT`.
 *
 * If `u < kc` over three subsequent calls to `bt_traverse()`, then
 * the internal stack of `bt_traverse()` will shrink its allocation.
 */
#define TRAVERSAL_STACK_MAJOR_USAGE_PERCENT ((long double).8)

typedef struct bt_node bt_node;

// `bt_node` stands for node_binary_tree.
typedef struct bt_node {
  size_t value_index;
  size_t parent_index;
  size_t left_index;
  size_t right_index;
} bt_node;

// Binary tree header.
typedef struct {
  stack *unused_nodes;  // Contains pointers to unused nodes.
  size_t allocation;    // Total bytes allocated for the tree and nodes.
  size_t padding;
  size_t root_index;
  size_t value_size;  // Size (in bytes) of a stored value.
} binary_tree;
/*
 * A note regarding the `padding` data member.
 *
 * If there are no unused nodes to be overwritten when adding a node to a tree,
 * new memory must be allocated to accomodate that new node.
 *
 * Unfortunately, with the current memory model, adding a node requires a shift
 * to the right of the nodes in that tree. The magnitude of this shift is equal
 * to the closest multiple of `8` greater than the tree's `value_size`. This is
 * to preserve the alignment of the nodes in memory.
 *
 * Examples of the aforementioned behavior:
 *
 * - If `value_size` is `3`, the shift magnitude will be `8`, since the closest
 * multiple of `8` with regards to `3` is `8`.
 * - If `value_size` is `29`, the shift magnitude will be `32`, since the
 * closest multiple of `8` with regards to `31` is `32`.
 *
 * This is done to create a gap of unused memory to contain that new node's
 * associated value. However, the amount of memory actually used for the value
 * is dictated by `value_size`, which is not guaranteed to align with the nodes.
 * Therefore, the nodes themselves will not fill in whatever gap remains.
 *
 * Examples of this behavior:
 * - If `value_size` is `3`, the nodes will be shifted to the right by `8`
 * bytes. However, only `3` bytes out of the `8`-byte gap were used for the
 * value, leaving `5` unused bytes.
 * - If `value_size` is `29`, the nodes will be shifted to the right by `32`
 * bytes. However, only `29` bytes out of the `32`-byte gap were used for the
 * value, leaving `3` unused bytes.
 *
 * TL;DR:
 * This value keeps track of the size of an unallocated region within the tree
 * that could be used for future node additions. If all you need is a binary
 * tree, don't worry about this.
 */

// This is a convenience macro for generating a binary tree from an array.
// Use caution if the arguments to this macro have side effects.
#define binary_tree_new(arr) \
  bt_new_(arr, sizeof *(arr), sizeof(arr) / sizeof *(arr))

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

/* Removes `target` and all its descendant nodes from `tree`. */
void bt_delete_node(binary_tree *tree, bt_node *target);

binary_tree *bt_expand(binary_tree *tree);

/*
 * Finds the closest descendant node of `origin` (if not `origin` itself) whose
 * `left` or `right` pointer is `NULL`.
 *
 * \return If `origin` has an either a `left` or `right` pointer, `origin` is
 * returned. Otherwise, a descendant node of `origin` meeting the same criteria
 * is returned.
 */
bt_node *bt_get_leaf(bt_node *origin);

bt_node **get_open_child_in_node(bt_node *node);

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
binary_tree *bt_resize(binary_tree *tree, size_t new_size);

/*
 * Traverses all of the descendant nodes of `origin`, including `origin` itself,
 * and passes `tree` and each traversed node as a pointer to `operation` and
 * `stop_condition` until `stop_condition` returns `true` or all the nodes in
 * the lineage of `origin` are contacted.
 *
 * This function is NOT recursive; it has a dependency on the `stack` data
 * structure implemented elsewhere in this library.
 *
 * The arguments for both `operation` and `stop_condition` will be an array
 * containing a pointer to the current traversed node and `tree`, respectively.
 * This pocket of memory will be of size `sizeof(binary_tree *) + sizeof(bt_node
 * *)`.
 *
 * \return The last returned value from `operation` or `NULL` if `operation` is
 * `NULL`.
 *
 * \note
 * - Calling this function with `stop_condition` equal to `NULL` will always
 * cause all descendant nodes of `origin` to be traversed. This can be used to
 * pre-allocate memory for the internal stack.
 *
 * - The data passed to `operation` is an array of a pointer to `tree` and a
 * pointer to the current traversed node. To access these nodes requires a
 * double dereference, not a single dereference.
 * That is, `*(type-name-1 **)tree_and_cur_node` accesses the first pointer and
 * `*(type-name-2 **)((type-name-1 **)tree_and_cur_node + 1)` accesses the
 * second pointer.
 *
 * - If a traversed node meets the criteria for `stop_condition` to return
 * `true`, `operation` will still be run for that node.
 *
 * - If the internal stack used for traversal does not use most of its
 * capacity after a few calls to this function (internal library calls count
 * towards this), it will shrink itself to reduce memory footprint.
 *
 * - `tree` is present only to permit tree-level operations or checks that
 * might be required of `operation` and `stop_condition`. It is not used in the
 * actual traversal and can be `NULL` if neither `operation` nor
 * `stop_condition` will make use of it.
 */
void bt_traverse(binary_tree *tree, bt_node *from,
                 bool (*operation)(binary_tree *tree, bt_node *node));

#endif
