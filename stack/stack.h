#ifndef STACK_H
#define STACK_H

#ifdef __STDC_VERSION__
#if (__STDC_VERSION__ >= 199901L)
#include <stdbool.h>
#else
typedef unsigned char bool;
#define true (1)
#define false (0)
#define inline
#endif
#else
typedef unsigned char bool;
#define true (1)
#define false (0)
#define inline
#endif

#include <stddef.h>

/*
 * `data` - Pointer to the contents of a stack.
 * `length` - The current length of the stack.
 * `capacity` - The number of values the stack can store before expansion is
 * necessary.
 * `value_size` - The size of each member value within the contents of the
 * stack.
 */
typedef struct {
  void *data;
  size_t length;
  size_t capacity;
  size_t value_size;
} stack;

#define stack_delete(stk) stack_delete_(&(stk))

#define stack_expand(stk) stack_expand(&(stk))

#define stack_init(type, capacity) stack_init_(sizeof(type), capacity)

#define stack_is_empty(stk) stack_is_empty_(&(stk))

/**
 * @brief Convenience macro for creating a new stack interface from data.
 *
 * @param data Pointer to the data.
 */
#define stack_interface(data) \
  stack_interface_(data, sizeof(data) / sizeof *(data), sizeof *(data))

/**
 * @brief Convenience macro for creating a new stack from data.
 *
 * @param data Pointer to the data.
 */
#define stack_new(data) \
  stack_new_(data, sizeof(data) / sizeof *(data), sizeof *(data))

#define stack_reset(stk) stack_reset_(&(stk))

#define stack_resize(stk) stack_resize_(&(stk))

#define stack_shrink(stk) stack_shrink_(&(stk))

#define stack_peek(stk) stack_peek_(&(stk))

#define stack_pop(stk) stack_pop_(&(stk))

#define stack_push(stk, elem) stack_push_(&(stk))

/**
 * @brief Deletes a stack.
 *
 * @param stk Pointer to the stack to be deleted.
 */
void stack_delete_(const stack *stk);

/**
 * @brief Expands the memory used by `stk->data`, thereby increasing its
 * capacity.
 *
 * @param stk Pointer to the stack.
 * @return `true` if the stack was expanded. `false` otherwise.
 */
bool stack_expand_(stack *stk);

/**
 * @brief Creates a new empty stack.
 *
 * @param num_elems Number of elements.
 * @param value_size Size of each element.
 * @return A stack capable of containing `capacity` values of size `value_size`.
 */
stack stack_init_(size_t value_size, size_t capacity);

/**
 * @brief Creates a stack header for the contents of `data`.
 *
 * @param data Pointer to the data.
 * @param length Length of the data.
 * @param value_size Size of each element.
 * @return A stack whose contents are that of `data`.
 * @note Any operations carried out on the stack may affect the contents stored
 * at `data`.
 */
stack stack_interface_(void *data, size_t length, size_t value_size);

bool stack_is_empty_(const stack *stk);

/**
 * @brief Creates a stack based off the elements in `data`.
 *
 * @param data Pointer to the data.
 * @param length Length of the data.
 * @param value_size Size of each element.
 * @return A new stack whose contents are a copy of `length` values at `data`.
 */
stack stack_new_(const void *data, size_t length, size_t value_size);

/**
 * @brief Resets `stk->length` to `0`.
 *
 * @param stk Pointer to the stack.
 */
void stack_reset_(stack *stk);

/**
 * @brief Resizes the memory used by `stk->data` to accommodate `new_capacity`
 * elements.
 *
 * @param stk Pointer to the stack.
 * @param new_capacity New capacity of the stack.
 * @return `true` if the stack was resized. `false` otherwise.
 */
bool stack_resize_(stack *stk, size_t new_capacity);

/**
 * @brief Shrinks the memory used by `stk->data` to `stk->used_capacity`.
 *
 * @param stk Pointer to the stack.
 * @return `true` if the stack was shrunk. `false` otherwise.
 */
bool stack_shrink_(stack *stk);

/**
 * @brief Returns, but does not remove, the top element of `stk`.
 *
 * @param stk Pointer to the stack.
 * @return Pointer to the top element in `stk` or NULL if the stack is empty.
 */
void *stack_peek_(const stack *stk);

/**
 * @brief Returns and removes the top element from `stk`.
 *
 * @param stk Pointer to the stack.
 * @return Pointer to the top element in `stk` or NULL if the stack is empty.
 */
void *stack_pop_(stack *stk);

/**
 * @brief Adds `elem` to `stk`, expanding if necessary. `elem` will then be the
 * new top element and will be returned by functions such as `stack_peek_()`.
 *
 * @param stk Pointer to the stack.
 * @param elem Pointer to the element to push.
 */
void stack_push_(stack *stk, const void *elem);

#include <string.h> /* For memcpy(). */

/* Ensures that each stack's allocation gets a unique name. */
#define GET_STACK_DATA_NAME(id) _stk_data_##id

/**
 * @brief Creates a stack of automatic storage duration.
 *
 * @param stk_id The identifier for the stack being assigned.
 * @param num_elems The maximum number of elements the stack will contain.
 * @param _elem_size The size of each element in the stack.
 * @note This is a macro. Use with caution if any of the arguments have side
 * effects.
 */
#define stack_auto_empty_new(stk_ident, capacity, type)           \
  {NULL, capacity, 0, sizeof(type)};                              \
  byte GET_STACK_DATA_NAME(stk_ident)[sizeof(type) * (capacity)]; \
  (stk_ident).data = GET_STACK_DATA_NAME(stk_ident)

/**
 * @brief Creates a stack of automatic storage duration whose contents are a
 * copy of the contents of `data`.
 *
 * @param stk_id The identifier for the stack being assigned.
 * @param num_elems The maximum number of elements the stack will contain.
 * @param _elem_size The size of each element in the stack.
 * @note This is a macro. Use with caution if any of the arguments have side
 * effects.
 */
#define stack_auto_new(stk_ident, data)                          \
  stack_auto_empty_new(stk_ident, sizeof(data) / sizeof *(data), \
                       sizeof *(data));                          \
  (stk_ident).length = sizeof(data) / sizeof *(data);            \
  memcpy(GET_STACK_DATA_NAME(stk_ident), data, sizeof(data));

/**
 * @brief Creates a stack of automatic storage duration which allocates memory
 * solely for the stack header (that is, the data members of `stack`).
 * `stack->data` will take the value of the pointer, `_data`.
 *
 * @param _data A pointer to the data to be interfaced.
 * @param num_elems The number of elements in `_data`.
 * @param _elem_size The size of each element in bytes.
 */
#define stack_auto_interface_new_(_data, length, type) \
  {(_data), length, length, sizeof(type)}

/**
 * @brief Convenience macro equivalent to `stack_auto_interface_new_()`.
 *
 * @param _data Pointer to an array.
 */
#define stack_auto_interface_new(_data)                             \
  stack_auto_interface_new_(_data, sizeof(_data) / sizeof *(_data), \
                            *(_data))

/**
 * @brief Adds `elem` to `stk` if space permits. `elem` will then be the new top
 * element and will be returned by functions such as `stack_peek_()`.
 *
 * @param stk Pointer to the stack.
 * @param elem Pointer to the element to push.
 * @note The word `auto` in this function corresponds to the storage duration of
 * the pointed-to stack.
 */
void stack_auto_push(stack *stk, const void *elem);
#endif
