#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * The stack `unallocated_nodes` used by binary trees to track unallocated nodes
 * is managed by this library, not the stack library, so we need functions that
 * guarantee not to modify the memory allocations made by this library while
 * still maintaining the operability of a stack.
 */
#define STACK_INCL_HEAPLESS_STACK

#include "../../stack/stack.h"
#include "../trees.h"

/*
 * Determines how many calls to `traverse_descendants()` where the internal
 * stack's `used_capacity` never exceeds the product of its `capacity` and
 * `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT` must be made before shrinking the
 * internal stack's allocated memory.
 */
static size_t TRAVERSAL_STACK_SHRINK_COUNTER_MAX = 3;

/*
 * If `traverse_descendants` is called `TRAVERSAL_STACK_SHRINK_COUNTER` times
 * and the `used_capacity` of the internal stack used by
 * `traverse_descendants()` is less than the product of this multiplier and the
 * stack's `capacity`, the internal stack will shrink its allocated memory.
 *
 * Expressed mathematically:
 *
 * c - The capacity of the stack.
 * u - The used capacity of the stack.
 * k - The value of `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT`.
 *
 * If `u < kc` over three subsequent calls to `traverse_descendants()`, then
 * the internal stack of `traverse_descendants()` will shrink its allocation.
 */
static double TRAVERSAL_STACK_MAJOR_USAGE_PERCENT = .8;

/* `node_bt` stands for node_binary_tree. */
typedef struct node_bt {
  void *value;
  struct node_bt *parent;
  struct node_bt *left, *right;
} node_bt;

/* Container structure for a binary tree data structure. */
typedef struct {
  node_bt *root;
  stack *unallocated_nodes; /* Contains pointers to unused nodes. */
  size_t num_nodes;
  size_t value_size;      /* Size (in bytes) of each node's stored values. */
  size_t allocation;      /* Total bytes allocated for the tree and nodes. */
  size_t used_allocation; /* Total bytes used from `allocation`. */
} binary_tree;

/*
 * This is a convenience macro for generating a `binary_tree` from an array.
 * Use caution if the arguments to this macro have side effects.
 */
#define new_binary_tree(arr) \
  _new_binary_tree(arr, sizeof(*data), sizeof(data) / sizeof(*data))

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
 * `->` - Signifies that the left-hand node is the parent of the right-hand
 * node.
 * `,`  - Separator.
 *
 * `(7) -> {(4) -> {(3), (0)}}`,
 * `(7) -> {(9) -> {(1), (0)}}`
 *
 * \return A balanced binary tree whose structure is in accordance with the
 * above example or `NULL` if the tree could not be created.
 * \note This function returns a `binary_tree` without any way of tracking
 * unused memory accrued after removing/deleting nodes.
 * See `init_open_nodes()` if you would like to enable a stack
 * (`unallocated_nodes`) to track unused memory.
 */
binary_tree *_new_binary_tree(const void *data, size_t elem_size,
                              size_t length);

/*
 * Adds the specified element as a child node of `parent_node`.
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 */
binary_tree *add_node_to_bt(binary_tree *tree, node_bt *node);

/*
 * Counts the number of descendant nodes linked to `origin`.
 *
 * \return The total amount of descendant nodes connected to `origin`.
 */
size_t count_descendant_nodes(node_bt *origin);

/*
 * Same as `delete_binary_tree()`, except this function will write `0` across
 * the allocated memory of `tree`.
 */
void delete_binary_tree(binary_tree **tree);

/*
 * Same as `delete_binary_tree()`, except this function will set all allocated
 * memory of the tree to zero, including pointers and tree/node statistics.
 */
void delete_binary_tree_s(binary_tree **tree);

void delete_node_and_lineage(binary_tree *tree, node_bt *target);

/*
 * Removes `target` from the hierarchy of `tree`, adding it to
 * `tree->unallocated_nodes` if `tree` accepts tracking of open blocks of
 * memory.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note Any child nodes of `target` will not be deleted. Instead, their new
 * parent will be somewhere in the lineage of `tree->parent`.
 */
binary_tree *delete_node_from_tree_s(binary_tree *tree, node_bt *target);

bool nodes_exist_in_same_tree(const node_bt *node_1, const node_bt *node_2);

binary_tree *expand_binary_tree(binary_tree *tree);

/*
 * Finds the first open slot (that is, a `left` or `right` pointer whose value
 * is `NULL`) in a branch of nodes.
 *
 * \return A pointer to an open slot.
 * \note If this function encounters a node whose `left` and `right` pointers
 * are both `NULL`, this function will return a pointer to the `left` pointer.
 */
node_bt **find_open_descendant(node_bt *origin);

/*
 * Finds the first open `left` or `right` pointer in `dst` and places `src`
 * there. If neither `left` or `right` are open, `dst->left` and its
 * descendants will be appended to the last open slot in the lineage of `src`
 * and `src` will overwrite `dst->left`.
 *
 * \note If `dst->left` must be appended to the lineage of `src` and a candidate
 * node in the descendants of `src` is found whose `left` or `right` pointer
 * values are `NULL`, the function will append the descendants of `dst` to the
 * availble pointer of that candidate node.
 */
void force_make_node_child_of(binary_tree *dst_tree, node_bt *dst,
                              binary_tree *src_tree, node_bt *src);

/*
 * Finds the longest lineage of `origin`. This function searches both the
 * `left` and `right` branches of `origin`.
 *
 * \return The maximum depth of `origin`.
 */
size_t get_depth(const node_bt *origin);

node_bt *get_open_node(binary_tree *tree);

size_t left_branch_depth(const node_bt *origin);

/*
 * Initializes a stack in `tree` which contains pointers to any open blocks of
 * memory left behind from removing or deleting nodes.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note The current implementation allocates this stack at the end of the
 * allocated memory for `tree`.
 */
binary_tree *init_open_nodes(binary_tree *tree);

/*
 * Finds the first open `left` or `right` pointer in `dst` and places `src`
 * there. If neither `left` or `right` are open, both `dst` and `src` will be
 * unmodified.
 *
 * \return A (potentially new) pointer associated with the contents of `dst`
 * or `NULL` upon failure.
 */
binary_tree *make_node_child_of(binary_tree *dst_tree, node_bt *dst,
                                binary_tree *src_tree, node_bt *src);

/*
 * Registers `open_node` as an open block of memory in `tree` for any new
 * incoming nodes to fill.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note If `init_open_nodes()` was not called prior to this function or
 * `tree->unallocated_nodes` is `NULL`, this function will fail and return
 * `NULL`. If this occurs, `tree` and `open_node` will be unchanged.
 */
binary_tree *push_unalloc_node(binary_tree *tree, node_bt *open_node);

/*
 * Searches along the ancestry of `origin` until a node is found whose `left`
 * and `right` pointers are not `NULL`. Such a node is considered divergent
 * since, during tree traversal, a search algorithm must choose either the
 * `left` or `right` branch of that node.
 *
 * \return A pointer to the first ancestral node of `origin` containing a branch
 * divergence or `NULL` if no suitable node is found.
 */
node_bt *next_ancestral_divergence(const node_bt *origin);

node_bt *remove_node_from_tree(binary_tree *tree, node_bt *target);

/*
 * Resizes the memory allocated for `tree` to `new_size`.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note If `new_size` is less than `tree->used_allocation`, then tree data
 * can be corrupted. If this can occur, consider using `resize_tree_s()`.
 */
binary_tree *resize_tree(binary_tree *tree, size_t new_size);

/*
 * Resizes the memory allocated for `tree` to `new_size`.
 *
 * For any node corrupted as a result of a resize, that node's parent will no
 * longer maintain a `left` or `right` pointer to that node. Instead, the
 * value of `left` or `right` associated with the corrupted node will be
 * `NULL`.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note Use caution when resizing a tree to a smaller size. If reallocation
 * fails and the requested tree size would corrupt nodes, this function will
 * still remove the would-be corrupted nodes from the tree.
 */
binary_tree *resize_tree_s(binary_tree *tree, size_t new_size);

size_t right_branch_depth(const node_bt *origin);

/*
 * Traverses all descendant nodes from `origin`, passing each encountered node
 * as a pointer to `op` until `stop_condition` returns `true` or all the nodes
 * in the lineage of `origin` are contacted.
 *
 * This function is NOT recursive; it has a dependency on the `stack` data
 * structure implemented elsewhere in this library.
 *
 * \return The returned value of `op` performed on the last traversed node or
 * `NULL` if `op` is `NULL`.
 * \note Calling this function with `stop_condition` equal to `NULL` will always
 * cause all descendant nodes of `origin` to be traversed. This can be used for
 * pre-allocating memory for the internal stack.
 * \note If the given node meets the criteria for `stop_condition`, `op` will
 * still be run for that node.
 * \note If the internal stack used for traversal does not use most of its
 * capacity after a few calls, it will shrink itself to reduce memory footprint.
 */
void *traverse_descendants(node_bt *origin, void *(*op)(node_bt *),
                           bool (*stop_condition)(node_bt *));
#endif
