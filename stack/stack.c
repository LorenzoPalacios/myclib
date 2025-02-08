#include "stack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// The factor by which to expand a stack's capacity.
#define STACK_EXPANSION_FACTOR (2)

static inline size_t alloc_calc(const stack *const stk, const size_t capacity) {
  return (capacity * stk->value_size) + sizeof(stack);
}

size_t stack_capacity(const stack *const stk) {
  return (stk->allocation - sizeof(stack)) / stk->value_size;
}

void stack_reset(stack *const stk) { stk->length = 0; }

void stack_clear(stack *const stk) {
  stk->length = 0;
  memset(stk->data, 0, stack_capacity(stk));
}

static inline byte *stack_data(stack *const stk) { return (byte *)(stk + 1); }

void stack_delete(stack *const stk) {
  free(stk);
}

stack *stack_expand(stack *const stk) {
  stack *new_stk = stack_resize(stk, STACK_EXPANSION_FACTOR * stk->allocation);
  /*
   * Reallocation may fail because the requested memory is too large. In this
   * case, we fall back to the safer, yet generally less efficient option of
   * only allocating enough memory for one new element in the stack.
   *
   * This has the side effect of requiring more later reallocations, but is
   * more likely to ensure stability. Of course, if this also fails, then
   * chances are the system is out of memory, so it's fine to return `NULL`.
   */
  if (new_stk == NULL)
    new_stk = stack_resize(stk, stk->allocation + stk->value_size);
  return new_stk;
}

static inline byte *stack_get_elem(stack *const stk, const size_t index) {
  return stk->data + (index * stk->value_size);
}

stack *stack_new_(const void *const data, const size_t value_cnt,
                  const size_t value_size) {
  stack *const stk = stack_empty_new(value_cnt, value_size);
  if (stk == NULL) return NULL;
  stk->length = value_cnt;

  for (size_t i = 0; i < value_cnt; i++) {
    memcpy(stk->data + (i * value_size), (const byte *)data + (i * value_size),
           value_size);
  }
  return stk;
}

stack *stack_empty_new(const size_t num_elems, const size_t value_size) {
  const size_t ALLOCATION = (num_elems * value_size) + sizeof(stack);
  stack *const stk = malloc(ALLOCATION);
  if (stk == NULL) return NULL;
  stk->data = stack_data(stk);
  stk->length = 0;
  stk->value_size = value_size;
  stk->allocation = ALLOCATION;
  return stk;
}

stack *stack_resize(stack *const stk, const size_t new_capacity) {
  const size_t ALLOCATION = alloc_calc(stk, new_capacity);
  stack *const new_stk = realloc(stk, ALLOCATION);
  if (new_stk == NULL) return NULL;
  new_stk->data = stack_data(new_stk);
  new_stk->allocation = ALLOCATION;

  if (new_capacity < new_stk->length) new_stk->length = new_capacity;

  return new_stk;
}

stack *stack_shrink_to_fit(stack *stk) {
  const size_t CAPACITY = stack_capacity(stk);
  if (CAPACITY > stk->length) stk = stack_resize(stk, stk->length);
  return stk;
}

void *stack_peek(stack *const stk) {
  if (stk->length == 0) return NULL;
  /*
   * Subtracting by one since `stk->length` is equivalent to the number of
   * elements within `stk`.
   */
  return stk->data + ((stk->length - 1) * stk->value_size);
}

void *stack_pop(stack *const stk) {
  if (stk->length == 0) return NULL;
  void *const val = stack_peek(stk);
  stk->length--;
  return val;
}

stack *stack_push(stack *stk, const void *const elem) {
  const size_t LENGTH = stk->length;
  if (LENGTH == stack_capacity(stk)) {
    stk = stack_expand(stk);
    if (stk == NULL) return NULL;
  }
  memcpy(stack_get_elem(stk, LENGTH), elem, stk->value_size);
  stk->length++;
  return stk;
}

// - HEAPLESS STACKS -

stack *stack_heapless_push(stack *const stk, const void *const elem) {
  const size_t LENGTH = stk->length;
  if (LENGTH == stack_capacity(stk)) return NULL;
  memcpy(stack_get_elem(stk, LENGTH), elem, stk->value_size);
  stk->length++;
  return stk;
}

// - INTERFACE STACKS -

stack *stack_interface_new_(void *const data, const size_t value_cnt,
                            const size_t value_size) {
  // The stack itself should not be managing any memory as it is an interface.
  // However, it should still have an allocation for its header.
  stack *const stk_interface = stack_empty_new(0, 0);
  if (stk_interface == NULL) return NULL;
  stk_interface->data = data;
  stk_interface->value_size = value_size;
  stk_interface->length = value_cnt;
  return stk_interface;
}

stack *stack_interface_push(stack *const stk, const void *const elem) {
  return stack_heapless_push(stk, elem);
}
