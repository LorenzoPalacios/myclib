#ifndef STACK_H
#define STACK_H

#include <stddef.h>

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L)

#if (__STDC_VERSION__ < 202311L)
#include <stdbool.h> /* For C99 to C17. */
#endif

#else

#define inline /* `inline` keyword is not standardized prior to C99. */
/* Boolean type for C95 and below. */
typedef unsigned char bool;
#ifndef true
#define true (1)
#endif
#ifndef false
#define false (0)
#endif

#endif

/* - CONVENIENCE MACROS - */

/*
 * A note on `stack_capacity` and `stack_height`:
 * The casts to `unsigned char` are deliberate. This avoids undefined behavior
 * by strict aliasing.
 */

#define stack_capacity(stk) (*((unsigned char *)(stk) - sizeof(size_t)))

#define stack_delete(stk) stack_delete_(&(stk))

#define stack_expand(stk) stack_expand_(&(stk))

#define stack_height(stk) (*((unsigned char *)(stk) - (2 * sizeof(size_t))))

#define stack_init(type, capacity) \
  stack_init_(sizeof((type)((unsigned char)0)), (size_t)(capacity))

#define stack_is_empty(stk) (stack_height(stk) == 0)

#define stack_new(arr) \
  stack_new_(arr, sizeof(arr) / sizeof *(arr), sizeof *(arr))

#define stack_reset(stk) (stack_height(stk) = 0)

#define stack_resize(stk, new_capacity) \
  stack_resize_(&(stk), (size_t)(new_capacity))

#define stack_shrink(stk) stack_shrink_(&(stk))

#define stack_peek(stk) \
  (stack_is_empty(stk) ? NULL : (stk) + (stack_height(stk) - 1))

#define stack_peek_s(stk) stack_peek_untyped(stk, sizeof(*(stk)))

#define stack_pop(stk) \
  (stack_is_empty(stk) ? NULL : (stk) + (--stack_height(stk)))

#define stack_pop_s(stk) stack_pop_untyped(stk, sizeof(*(stk)))

#define stack_push(stk, value) stack_push_(&(stk), value)

/* - DEFINITIONS - */

#define stack(type) type *

/* - FUNCTIONS - */

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
bool stack_expand_(void **stk);

void *stack_header(void *stk);

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

inline void *stack_peek_untyped(void *stk, size_t value_size);

inline void *stack_pop_untyped(void *stk, size_t value_size);

/**
 * @brief Resizes the memory used by a stack to accommodate
 * `new_capacity` elements.
 *
 * @param stk The stack to be resized.
 * @param new_capacity The new capacity of the stack.
 * @return `true` if the stack was successfully resized.
 * `false` otherwise.
 */
bool stack_resize_(void **stk, size_t new_capacity);

/**
 * @brief Shrinks the memory used by a stack to the minimum
 * necessary to preserve data.
 *
 * @param stk The stack to be shrunk.
 * @return `true` if the stack was successfully shrunk.
 * `false` otherwise.
 */
bool stack_shrink_(void **stk);

/**
 * @brief Pushes `value` onto the top of a stack.
 *
 * @param stk The stack to push an element upon.
 * @param value Pointer to the value to be pushed.
 * @return `true` if the value was successfully pushed onto
 * the stack. `false` otherwise.
 */
bool stack_push_(void **stk, const void *value);
#endif
