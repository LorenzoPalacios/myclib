#ifndef TREES_H
#define TREES_H

#include <stddef.h>

#include "binarytree/binarytree.h"

#define delete_tree(tree) \
  (_Generic((tree), binary_tree *: delete_binary_tree)(&(tree)))
#define delete_tree_s(tree) \
  (_Generic((tree), binary_tree *: delete_binary_tree_s)(&(tree)))
#define add_node(tree, node) \
  (_Generic(tree, binary_tree *: add_binary_node)(tree, node))
#define remove_node(tree, node) \
  (_Generic(tree, binary_tree *: remove_binary_node)(tree, node))

#endif
