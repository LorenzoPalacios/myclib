#ifndef STACK_H
#define STACK_H

/*
 * Define `STACK_WANT_HEAPLESS` to include support for stacks of automatic
 * storage.
 */

#include <stddef.h>

typedef unsigned char byte;

/**
 * @brief Structure representing a stack.
 */
typedef struct {
  byte *data;         /**< Pointer to the stack data. */
  size_t length;      /**< Number of elements in the stack. */
  size_t elem_size;   /**< Size of each element in the stack. */
  size_t allocation;  /**< Total allocated memory for the stack. */
} stack;

/**
 * @brief Macro to delete a stack and free its memory.
 *
 * @param stk Pointer to the stack.
 */
#define stack_delete(stk) stack_delete_(&(stk))

/**
 * @brief Macro to securely delete a stack by zeroing its memory before freeing.
 *
 * @param stk Pointer to the stack.
 */
#define stack_delete_s(stk) stack_delete_s_(&(stk))

/**
 * @brief Convenience macro for creating a new stack from data.
 *
 * @param data Pointer to the data.
 */
#define stack_new(data) \
  stack_new_(data, sizeof(data) / sizeof *(data), sizeof *(data))

/**
 * @brief Convenience macro for creating a new stack interface from data.
 *
 * @param data Pointer to the data.
 */
#define new_interface_stack(data) \
  stack_interface_new_(data, sizeof(data) / sizeof *(data), sizeof *(data))

/**
 * @brief Creates a stack based off the elements in `data`.
 *
 * @param data Pointer to the data.
 * @param len Length of the data.
 * @param elem_size Size of each element.
 * @return Pointer to the new stack or NULL if allocation fails.
 */
stack *stack_new_(const void *data, size_t len, size_t elem_size);

/**
 * @brief Gets the capacity of the stack.
 *
 * @param stk Pointer to the stack.
 * @return Capacity of the stack.
 */
size_t stack_capacity(const stack *stk);

/**
 * @brief Resets `stk->length` to `0`.
 *
 * @param stk Pointer to the stack.
 */
void stack_reset(stack *stk);

/**
 * @brief Resets the stack and sets all elements to zero.
 *
 * @param stk Pointer to the stack.
 */
void stack_clear(stack *stk);

/**
 * @brief Deletes the stack and frees memory.
 *
 * @param stk Double pointer to the stack.
 */
void stack_delete_(stack **stk);

/**
 * @brief Deletes the stack, sets all elements to zero, and frees memory.
 *
 * @param stk Double pointer to the stack.
 */
void stack_delete_s_(stack **stk);

/**
 * @brief Expands the memory used by `stk->data`, thereby increasing its capacity.
 *
 * @param stk Pointer to the stack.
 * @return Pointer to the expanded stack or NULL if reallocation fails.
 */
stack *stack_expand(stack *stk);

/**
 * @brief Creates a new empty stack.
 *
 * @param num_elems Number of elements.
 * @param elem_size Size of each element.
 * @return Pointer to the new stack or NULL if allocation fails.
 */
stack *stack_empty_new(size_t num_elems, size_t elem_size);

/**
 * @brief Creates a stack header which interfaces upon the contents of `data`.
 *
 * @param data Pointer to the data.
 * @param len Length of the data.
 * @param elem_size Size of each element.
 * @return Pointer to the stack header or NULL if allocation fails.
 * @note Any operations carried out on the stack may affect the contents stored
 * at `data`. This is in contrast to the standard stack which allocates distinct
 * memory for its contents. An example of such an operation would be
 * `stack_interface_push()`.
 */
stack *stack_interface_new_(void *data, size_t len, size_t elem_size);

/**
 * @brief Resizes the memory used by `stk->data` to accommodate `new_capacity` elements.
 *
 * @param stk Pointer to the stack.
 * @param new_capacity New capacity of the stack.
 * @return Pointer to the resized stack or NULL if reallocation fails.
 */
stack *stack_resize(stack *stk, size_t new_capacity);

/**
 * @brief Shrinks the memory used by `stk->data` to `stk->used_capacity`.
 *
 * @param stk Pointer to the stack.
 * @return Pointer to the resized stack or NULL if reallocation fails.
 */
stack *stack_shrink_to_fit(stack *stk);

/**
 * @brief Returns, but does not remove, the top element of `stk`.
 *
 * @param stk Pointer to the stack.
 * @return Pointer to the top element in `stk` or NULL if the end of the
 * stack was reached.
 */
void *stack_peek(stack *stk);

/**
 * @brief Returns and removes the top element from `stk`.
 *
 * @param stk Pointer to the stack.
 * @return Pointer to the top element in `stk` or NULL if the end of the
 * stack was reached.
 */
void *stack_pop(stack *stk);

/**
 * @brief Adds `elem` to `stk`, expanding if necessary. `elem` will then be the new top
 * element and will be returned by functions such as `stack_peek()`.
 *
 * @param stk Pointer to the stack.
 * @param elem Pointer to the element to push.
 * @return Pointer to the stack or NULL if reallocation fails.
 */
stack *stack_push(stack *stk, const void *elem);

#ifdef STACK_WANT_HEAPLESS
#include <string.h> // For memcpy().

#define GET_LINE __LINE__
#define STRINGIFY_LINE(line) #line
// Ensures that each stack's allocation gets a unique name.
#define GET_STACK_NAME(id) _stk_data_ ## id

/**
 * @brief Creates a stack with automatic storage duration.
 *
 * @param stk_id The identifier for the stack being assigned.
 * @param num_elems The maximum number of elements the stack will contain.
 * @param _elem_size The size of each element in the stack.
 * @note This is a macro. Use with caution if any of the arguments have side
 * effects.
 */
#define stack_heapless_empty_new(stk_id, num_elems, _elem_size) \
  {.allocation = ((num_elems) * (_elem_size)) + sizeof(stack),  \
   .elem_size = (_elem_size),                                   \
   .length = 0};                                                \
  byte GET_STACK_NAME(stk_id)[(num_elems) * (_elem_size)];      \
  (stk_id).data = GET_STACK_NAME(stk_id)

/**
 * @brief Creates a stack with automatic storage duration whose contents are a copy of
 * the contents of `data`.
 *
 * @param stk_id The identifier for the stack being assigned.
 * @param num_elems The maximum number of elements the stack will contain.
 * @param _elem_size The size of each element in the stack.
 * @note This is a macro. Use with caution if any of the arguments have side
 * effects.
 */
#define stack_heapless_new(stk_id, data)                          \
  stack_heapless_empty_new(stk_id, sizeof(data) / sizeof *(data), \
                           sizeof *(data));                       \
  (stk_id).length = sizeof(data) / sizeof *(data);                \
  memcpy(GET_STACK_NAME(stk_id), data, sizeof(data));

/**
 * @brief Creates a stack of automatic storage duration which allocates memory solely
 * for the stack header (that is, the data members of `stack`). `stack->data`
 * will take the value of the pointer, `data`.
 *
 * @param _data A pointer to the data to be interfaced.
 * @param num_elems The number of elements in `_data`.
 * @param _elem_size The size of each element in bytes.
 *
 * @note This stack will not modify the allocation of memory at `data`,
 * however it can modify the contents of `data` through `heapless_stack_pop()`
 * and `heapless_stack_push()`.
 */
#define stack_heapless_interface_new_(_data, num_elems, _elem_size) \
  {.data = (byte *)(_data),                                         \
   .allocation = ((num_elems) * (_elem_size)) + sizeof(stack),      \
   .elem_size = (_elem_size),                                       \
   .length = (num_elems)}

/**
 * @brief Convenience macro equivalent to `heapless_new_interface_stack_()`.
 *
 * @param _data Pointer to an array.
 */
#define stack_heapless_interface_new(_data)                             \
  stack_heapless_interface_new_(_data, sizeof(_data) / sizeof *(_data), \
                                sizeof *(_data))

/**
 * @brief Adds `elem` to `stk` if space permits. `elem` will then be the new top
 * element and will be returned by functions such as `heapless_stack_peek()`.
 *
 * @param stk Pointer to the stack.
 * @param elem Pointer to the element to push.
 * @return `stk` if the element was added successfully or `NULL` upon failure.
 */
stack *stack_heapless_push(stack *stk, const void *elem);
#endif
#endif
