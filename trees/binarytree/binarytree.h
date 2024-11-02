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
 * Determines how many calls to `traverse_from()` where the internal
 * stack's `used_capacity` never exceeds the product of its `capacity` and
 * `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT` must be made before shrinking the
 * internal stack's allocated memory.
 */
static size_t TRAVERSAL_STACK_SHRINK_COUNTER_MAX = 5;

/*
 * If `traverse_from` is called `TRAVERSAL_STACK_SHRINK_COUNTER` times
 * and the `used_capacity` of the internal stack used by
 * `traverse_from()` is less than the product of this multiplier and the
 * stack's `capacity`, the internal stack will shrink its allocated memory.
 *
 * Expressed mathematically:
 *
 * c - The capacity of the stack.
 * u - The used capacity of the stack.
 * k - The value of `TRAVERSAL_STACK_MAJOR_USAGE_PERCENT`.
 *
 * If `u < kc` over three subsequent calls to `traverse_from()`, then
 * the internal stack of `traverse_from()` will shrink its allocation.
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
  size_t value_size;        /* Size (in bytes) of each node's stored values. */
  size_t allocation;        /* Total bytes allocated for the tree and nodes. */
  size_t used_allocation;   /* Total bytes used from `allocation`. */
} binary_tree;

/*
 * This is a convenience macro for generating a `binary_tree` from an array.
 * Use caution if the arguments to this macro have side effects.
 */
#define new_binary_tree(arr) \
  _new_binary_tree(arr, sizeof(*arr), sizeof(arr) / sizeof(*arr))

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
 * \note This function returns a `binary_tree` without any way of tracking
 * unused memory accrued after removing/deleting nodes.
 * See `init_unalloc_nodes_stk()` if you would like to enable a stack
 * (`unallocated_nodes`) to track unused memory.
 */
binary_tree *_new_binary_tree(const void *data, size_t elem_size,
                              size_t length);

/*
 * Adds the specified discrete node `node` to `tree`. On success, the pointer to
 * `node` will be updated to its new position in `tree` and the original pointer
 * to `node` invalidated.
 *
 * It is assumed that the data at `node->value` is of size equivalent to
 * `tree->value_size`. If this assumption is false, the behavior is undefined.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note If `node` is a node within a binary tree, the behavior is undefined.
 */
binary_tree *add_freestanding_node(binary_tree *tree, node_bt **node);

/*
 * Counts the number of descendant nodes linked to `origin`.
 *
 * \return The total amount of descendant nodes connected to `origin`.
 */
size_t count_descendant_nodes(node_bt *origin);

/*
 * Creates a discrete binary tree node with no ties to any trees, child nodes,
 * or parent nodes.
 *
 * \return A pointer to a discrete binary tree node or `NULL` upon failure.
 */
node_bt *new_bt_node(const void *value, size_t value_size);

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

/* Removes `target` and all its descendant nodes from `tree`. */
void delete_node(binary_tree *tree, node_bt *target);

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
binary_tree *delete_node_from_tree(binary_tree *tree, node_bt *target);

binary_tree *expand_binary_tree(binary_tree *tree);

/*
 * Finds the closest descendant node of `origin` (if not `origin` itself) whose
 * `left` or `right` pointer is `NULL`.
 *
 * \return If `origin` has an either a `left` or `right` pointer, `origin` is
 * returned. Otherwise, a descendant node of `origin` meeting the same criteria
 * is returned.
 */
node_bt *find_open_descendant(node_bt *origin);

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
binary_tree *force_make_node_child_of(binary_tree *dst_tree, node_bt *dst,
                                      binary_tree *src_tree, node_bt *src);

/*
 * Finds the longest lineage of `origin`. This function searches both the
 * `left` and `right` branches of `origin`.
 *
 * \return The maximum depth of `origin`.
 */
size_t get_depth(const node_bt *origin);

node_bt **get_open_child_in_node(node_bt *const node);

node_bt *get_unalloc_node(binary_tree *tree);

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
binary_tree *init_unalloc_nodes_stk(binary_tree *tree);

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
 * \note If `init_unalloc_nodes_stk()` was not called prior to this function or
 * `tree->unallocated_nodes` is `NULL`, this function will fail and return
 * `NULL`. If this occurs, `tree` and `open_node` will be unchanged.
 */
binary_tree *register_unalloc_node(binary_tree *tree, node_bt *open_node);

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

bool do_nodes_coexist_in_tree(const node_bt *node_1, const node_bt *node_2);

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
 * Traverses all of the descendant nodes of `origin`, including `origin` itself,
 * amd passes each traversed node as a pointer to `op` and `stop_condition`
 * until `stop_condition` returns `true` or all the nodes in the lineage of
 * `origin` are contacted.
 *
 * This function is NOT recursive; it has a dependency on the `stack` data
 * structure implemented elsewhere in this library.
 *
 * The arguments for both `op` and `stop_condition` will be a pocket of memory
 * containing the pointers `tree` and `origin`, respectively. This pocket of
 * memory will be of size `sizeof(binary_tree *) + sizeof(node_bt *)`.
 *
 * \return The last returned value from `op` or `NULL` if `op` is `NULL`.
 * \note Calling this function with `stop_condition` equal to `NULL` will always
 * cause all descendant nodes of `origin` to be traversed. This can be used for
 * pre-allocating memory for the internal stack.
 *
 * \note If a traversed node meets the criteria for `stop_condition` to return
 * `true`, `op` will still be run for that node.
 *
 * \note If the internal stack used for traversal does not use most of its
 * capacity after a few calls to this function (internal library calls count
 * towards this), it will shrink itself to reduce memory footprint.
 *
 * \note `tree` is present only to permit tree-level operations or checks that
 * might be required of `op` and `stop_condition`. It is not used in the actual
 * traversal and can be `NULL` if neither `op` nor `stop_condition` will make
 * use of it.
 */
void *traverse_from(binary_tree *tree, node_bt *origin,
                    void *(*op)(void *tree_and_cur_node),
                    bool (*stop_condition)(void *tree_and_cur_node));

#endif
