#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <stddef.h>
#include <vadefs.h>

#include "../trees.h"

typedef struct bt_node {
  void *value;
  struct bt_node *parent;
  struct bt_node *left, *right;
} bt_node;

/* Container structure for a binary tree data structure. */
typedef struct {
  bt_node *root;
  bt_node **open_nodes;
  size_t num_nodes;
  size_t node_size;
  size_t allocation; /* Total bytes allocated for the tree and nodes. */
  size_t used_allocation;
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
 * \return
 * \note This function returns a `binary_tree` without any way of tracking
 * unused memory accrued after removing/deleting nodes.
 * See `init_open_nodes()` if you would like to enable a stack (`open_nodes`)
 * to track unused memory.
 */
binary_tree *_new_binary_tree(const void *data, size_t elem_size,
                              size_t length);

/*
 * Adds the specified element as a child node of `parent_node`.
 * \return A pointer to the added node, or NULL upon failure.
 */
bt_node *add_binary_node(binary_tree *tree, const void *elem);

/*
 * Registers `open_node` as an open block of memory in `tree` for any new
 * incoming nodes to fill.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note If `init_open_nodes()` was not called prior to this function or
 * `tree->open_nodes` is `NULL`, this function will fail and return `NULL`.
 * If this occurs, `tree` and `open_node` will be unchanged.
 */
binary_tree *add_open_node(binary_tree *tree, bt_node *open_node);

/*
 * Calculates the number of descendant nodes linked to `origin`.
 *
 * \return The total amount of descendant nodes connected to `origin`.
 */
size_t count_descendant_nodes(bt_node *origin);

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

void delete_node_and_lineage(binary_tree *tree, bt_node *target);

/*
 * Removes `target` from the hierarchy of `tree`, adding it to
 * `tree->open_nodes` if `tree` accepts tracking of open blocks of memory.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note Any child nodes of `target` will not be deleted. Instead, their new
 * parent will be somewhere in the lineage of `tree->parent`.
 */
binary_tree *delete_node_from_tree_s(binary_tree *tree, bt_node *target);

binary_tree *expand_tree(binary_tree *tree);

/*
 * Finds the first open slot (that is, a `left` or `right` pointer whose value
 * is `NULL`) in a branch of nodes.
 *
 * \return A pointer to an open slot.
 * \note If this function encounters a node whose `left` and `right` pointers
 * are both `NULL`, this function will return a pointer to the `left` pointer.
 */
bt_node **find_open_descendant(bt_node *origin);

/*
 * Finds the first open `left` or `right` pointer in `dst` and places `src`
 * there. If neither `left` or `right` are open, `dst->left` and its
 * descendants will be appended to the last open slot in the lineage of `src`
 * and `src` will overwrite `dst->left`.
 *
 * \note If `dst->left` must be appended to the lineage of `src` and a candidate
 * node in the descendants of `src` is found whose `left` and `right` pointer
 * values are `NULL`, the function will append the descendants of `dst` to the
 * `left` pointer of that candidate node.
 */
void force_make_node_child_of(bt_node *const src, bt_node *const dst);

/*
 * Finds the longest lineage of `origin`. This function searches both the
 * `left` and `right` branches of `origin`.
 *
 * \return The maximum depth of `origin`.
 */
size_t get_depth(bt_node *origin);

bt_node *get_open_node(binary_tree *tree);

void operate_over_lineage(bt_node *origin,
                          void (*op)(bt_node *node, va_list *args),
                          va_list *args);

/*
 * Initializes a rudimentary stack in `tree` which contains pointers to any open
 * blocks of memory left behind from removing or deleting nodes.
 *
 * This can also be used to reset a stack of open nodes, although if this
 * function is called with a binary tree whose `open_nodes` is not yet
 * exhausted, memory leaks can occur as the pointers to any open blocks of
 * memory may be lost.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note The current implementation allocates this stack at the end of the
 * allocated memory for `tree`. Additionally, if this function returns `NULL`,
 * then no stack has been allocated.
 */
binary_tree *init_open_nodes(binary_tree *tree);

/*
 * Finds the first open `left` or `right` pointer in `dst` and places `src`
 * there. If neither `left` or `right` are open, both `dst` and `src` will be
 * unmodified.
 */
void make_node_child_of(bt_node *src, bt_node *dst);

bt_node *next_ancestral_divergence(const bt_node *origin);

bt_node *remove_node_from_tree(binary_tree *tree, bt_node *target);

/*
 * Resizes the memory allocated for `tree` to `new_size`.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note If `new_size` is less than `tree->used_allocation`, then tree data can
 * be corrupted. If this can occur, consider using `resize_tree_s()`.
 */
binary_tree *resize_tree(binary_tree *tree, size_t new_size);

/*
 * Resizes the memory allocated for `tree` to `new_size`.
 *
 * For any node corrupted as a result of a resize, that node's parent will no
 * longer maintain a `left` or `right` pointer to that node. Instead, the value
 * of `left` or `right` associated with the corrupted node will be `NULL`.
 *
 * \return A (potentially new) pointer associated with the contents of `tree`
 * or `NULL` upon failure.
 * \note Use caution when resizing a tree to a smaller size. If reallocation
 * fails and the requested tree size would corrupt nodes, this function will
 * still remove the would-be corrupted nodes from the tree.
 */
binary_tree *resize_tree_s(binary_tree *tree, size_t new_size);

bt_node **search_for_node(bt_node *origin, const bt_node *target);
#endif
