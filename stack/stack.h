#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdlib.h>

#include "../include/boolmyclib.h"

/* - DEFINITIONS - */

#define stack(type) type *

/* The factor by which to expand a stack's capacity. */
#define STK_EXPANSION_FACTOR ((size_t)2)

/* - CONVENIENCE MACROS - */

#define stack_capacity(stk) (*stack_capacity_((void *)(stk)))

#define stack_copy(stk) \
  stack_new_((const void *)(stk), stack_capacity(stk), sizeof *(stk))

#define stack_delete(stk)            \
  free(stack_header((void *)(stk))); \
  (stk) = NULL

#define stack_delete_s(stk) stack_delete_((void **)&(stk))

#define stack_expand(stk)                                        \
  (stack_resize(stk, STK_EXPANSION_FACTOR * stack_capacity(stk)) \
       ? true                                                    \
       : stack_resize(stk, stack_capacity(stk) + 1))

#define stack_expand_s(stk) stack_expand_((void **)&(stk), sizeof *(stk))

#define stack_header(stk) stack_header_((void *)(stk))

#define stack_height(stk) (*stack_height_((void *)(stk)))

#define stack_init(type, capacity) \
  ((type *)stack_init_(sizeof *((type *)NULL), (size_t)(capacity)))

#define stack_is_full(stk) (stack_height(stk) == stack_capacity(stk))

#define stack_is_full_s(stk) stack_is_full_((void *)(stk))

#define stack_is_empty(stk) (stack_height(stk) == 0)

#define stack_new(arr) \
  stack_new_((const void *)(arr), sizeof(arr) / sizeof *(arr), sizeof *(arr))

#define stack_peek(stk) \
  (stack_is_empty(stk) ? NULL : (stk) + (stack_height(stk) - 1))

#define stack_peek_s(stk) stack_peek_untyped((void *)(stk), sizeof *(stk))

#define stack_pop(stk) \
  (stack_is_empty(stk) ? NULL : (stk) + (--(stack_height(stk))))

#define stack_pop_s(stk) stack_pop_untyped((void *)(stk), sizeof *(stk))

#define stack_push(stk, value)                                    \
  (stack_height(stk) == stack_capacity(stk) && !stack_expand(stk) \
       ? false                                                    \
       : ((void)((stk)[(stack_height(stk))++] = *(value)), true))

#define stack_push_s(stk, value) \
  stack_push_((void **)&(stk), value, sizeof *(stk))

#define stack_reset(stk) (stack_height(stk) = 0)

#define stack_reset_s(stk) stack_reset_((void *)(stk))

#define stack_resize(stk, new_capacity) \
  stack_resize_((void **)&(stk), (size_t)(new_capacity), sizeof *(stk))

#define stack_shrink(stk) stack_resize(stk, stack_height(stk))

#define stack_shrink_s(stk) stack_shrink_((void **)&(stk), sizeof *(stk))

/*
 * Some of the provided macros rely upon a function that operates with pointers
 * to void type, which may cause linter warnings or compilation errors depending
 * on usage.
 *
 * These pointers can be casted to appropriate types under the C23 standard with
 * the `typeof` operator, which is the purpose of the following preprocessor
 * section.
 */
#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202311)
#undef stack_copy
#define stack_copy(stk)                                             \
  (typeof(stk))stack_new_((const void *)(stk), stack_capacity(stk), \
                          sizeof *(stk))

#undef stack_new
#define stack_new(arr)                  \
  ((typeof_unqual(*(arr)) *)stack_new_( \
      (const void *)(arr), sizeof(arr) / sizeof *(arr), sizeof *(arr)))

#undef stack_peek_s
#define stack_peek_s(stk) \
  (typeof(stk))stack_peek_untyped((void *)(stk), sizeof *(stk))

#undef stack_pop_s
#define stack_pop_s(stk) \
  (typeof(stk))stack_pop_untyped((void *)(stk), sizeof *(stk))

#endif

/* - FUNCTIONS - */

size_t *stack_capacity_(void *stk);

/**
 * @brief Deletes a stack.
 *
 * @param stk The stack to be deleted.
 */
void stack_delete_(void **stk);

/**
 * @brief Expands the capacity of a stack.
 *
 * @param stk The stack to be expanded.
 * @return `true` if the stack was successfully expanded.
 * `false` otherwise.
 */
bool stack_expand_(void **stk, size_t value_size);

void *stack_header_(void *stk);

size_t *stack_height_(void *stk);

/**
 * @brief Creates a new and empty stack.
 *
 * @param capacity The number of values the stack should be
 * capable of holding.
 * @param value_size The size of each value in the stack.
 * @return A stack capable of containing `capacity` values
 * of size `value_size`.
 */
void *stack_init_(size_t value_size, size_t capacity);

bool stack_is_full_(void *stk);

/**
 * @brief Creates a stack based off the elements in `data`.
 *
 * @param data Pointer to the data.
 * @param length Length of the data.
 * @param value_size Size of each element.
 * @return A new stack whose contents are a copy of `length`
 * values at `data`.
 */
void *stack_new_(const void *data, size_t length, size_t value_size);

void *stack_peek_untyped(void *stk, size_t value_size);

void *stack_pop_untyped(void *stk, size_t value_size);

/**
 * @brief Resizes the memory used by a stack to accommodate
 * `new_capacity` elements.
 *
 * @param stk The stack to be resized.
 * @param new_capacity The new capacity of the stack.
 * @return `true` if the stack was successfully resized.
 * `false` otherwise.
 */
bool stack_resize_(void **stk, size_t new_capacity, size_t value_size);

void stack_reset_(void *stk);

/**
 * @brief Shrinks the memory used by a stack to the minimum
 * necessary to preserve data.
 *
 * @param stk The stack to be shrunk.
 * @return `true` if the stack was successfully shrunk.
 * `false` otherwise.
 */
bool stack_shrink_(void **stk, size_t value_size);

/**
 * @brief Pushes `value` onto the top of a stack.
 *
 * @param stk The stack to push an element upon.
 * @param value Pointer to the value to be pushed.
 * @return `true` if the value was successfully pushed onto
 * the stack. `false` otherwise.
 */
bool stack_push_(void **stk, const void *value, size_t value_size);
#endif
