#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The factor by which to expand a stack's capacity. */
#define STACK_EXPANSION_FACTOR (2)

stack *_new_stack(const void *const data, const size_t len,
                  const size_t elem_size) {
  stack *const stk = new_empty_stack(len, elem_size);
  if (stk == NULL) return NULL;
  stk->length = len;
  stk->used_capacity = stk->capacity;
  /* 
   * Elements of `data` are inserted in reverse so that the first element of
   * `data` will always be the top-level element in the stack.
   */
  for (size_t stk_i = 0, data_i = len - 1; stk_i < len; stk_i++, data_i--) {
    memcpy((byte_t *)stk->data + stk_i * elem_size,
           (byte_t *)data + data_i * elem_size, elem_size);
  }
  return stk;
}

static stack *alloc_stack(const size_t stack_capacity) {
  return malloc(stack_capacity + sizeof(stack));
}

void clear_stack(stack *stk) {
  stk->length = 0;
  stk->used_capacity = 0;
}

void clear_stack_s(stack *stk) {
  stk->length = 0;
  stk->used_capacity = 0;
  memset(stk->data, 0, stk->capacity);
}

void delete_stack(stack **const stk) {
  free(*stk);
  *stk = NULL;
}

void delete_stack_s(stack **const stk) {
  memset(*stk, 0, (*stk)->capacity);
  delete_stack(stk);
}

stack *expand_stack(stack *stk) {
  stack *new_stk = resize_stack(stk, STACK_EXPANSION_FACTOR * stk->capacity);
  /*
   * Reallocation may fail because the requested memory is too large.
   * In this case, we fall back to the safer, yet generally less efficient
   * option of only allocating enough memory for one new element in the stack.
   * This has the likely side effect of requiring more later reallocations, but
   * is more likely to ensure program stability.
   * Of course, if this also fails, then chances are the system is out of
   * memory, so it's fine to return `NULL`.
   */
  if (new_stk == NULL)
    new_stk = resize_stack(stk, stk->capacity + stk->elem_size);
  return new_stk;
}

stack *new_empty_stack(const size_t num_elems, const size_t elem_size) {
  const size_t STACK_CAPACITY = num_elems * elem_size;
  stack *const stk = alloc_stack(STACK_CAPACITY);
  if (stk == NULL) return NULL;
  stk->capacity = STACK_CAPACITY;
  stk->used_capacity = 0;
  stk->elem_size = elem_size;
  stk->data = stk + 1; /* Increment past the stack header. */
  stk->length = 0;
  return stk;
}

stack *resize_stack(stack *stk, size_t new_size) {
  {
    const size_t SIZE_DIFF = new_size % stk->elem_size;
    if (SIZE_DIFF != 0) new_size += stk->elem_size - SIZE_DIFF;
  }
  stk = realloc(stk, new_size + sizeof(stack));
  if (stk == NULL) return NULL;
  stk->capacity = new_size;
  stk->data = stk + 1; /* Increment past the stack header. */
  return stk;
}

stack *shrink_stack_to_fit(stack *stk) {
  if (stk->capacity > stk->used_capacity)
    stk = resize_stack(stk, stk->used_capacity);
  return stk;
}

void *stack_peek(stack *const stk) {
  if (stk->length == 0) return NULL;
  /*
   * Subtracting by one since `stk->length` is equivalent to the number of
   * elements within `stk`.
   */
  return (byte_t *)stk->data + (stk->length - 1) * stk->elem_size;
}

void *stack_pop(stack *const stk) {
  if (stk->length == 0) return NULL;
  void *val = stack_peek(stk);
  stk->length--;
  stk->used_capacity -= stk->elem_size;
  return val;
}

stack *stack_push(stack *stk, const void *const elem) {
  if (stk->used_capacity + stk->elem_size > stk->capacity) {
    stk = expand_stack(stk);
    if (stk == NULL) return NULL;
  }
  const size_t LENGTH = stk->length;
  const size_t ELEM_SIZE = stk->elem_size;
  memcpy((byte_t *)stk->data + LENGTH * ELEM_SIZE, elem, ELEM_SIZE);
  stk->length++;
  stk->used_capacity += stk->elem_size;
  return stk;
}

/* For heapless stacks. */

void *heapless_stack_peek(stack *const stk) { return stack_peek(stk); }

void *heapless_stack_pop(stack *const stk) { return stack_pop(stk); }

stack *heapless_stack_push(stack *const stk, const void *const elem) {
  if (stk->used_capacity + stk->elem_size > stk->capacity) return NULL;
  const size_t LENGTH = stk->length;
  const size_t ELEM_SIZE = stk->elem_size;
  memcpy((byte_t *)stk->data + LENGTH * ELEM_SIZE, elem, ELEM_SIZE);
  stk->length++;
  stk->used_capacity += stk->elem_size;
  return stk;
}

/* For stacks as interfaces. */

stack *_new_interface_stack(void *const data, const size_t len,
                            const size_t elem_size) {
  /*
   * The stack itself should not be managing any memory as it is an interface.
   * However, it should still have an allocation for its header.
   */
  stack *const stk_interface = alloc_stack(0);
  if (stk_interface == NULL) return NULL;
  stk_interface->data = data;
  stk_interface->capacity = len * elem_size;
  stk_interface->used_capacity = stk_interface->capacity;
  stk_interface->elem_size = elem_size;
  stk_interface->length = len;
  return stk_interface;
}

void *interface_stack_pop(stack *const stk) { return stack_pop(stk); }

void *interface_stack_peek(stack *const stk) { return stack_peek(stk); }

stack *interface_stack_push(stack *const stk, const void *const elem) {
  return heapless_stack_push(stk, elem);
}
