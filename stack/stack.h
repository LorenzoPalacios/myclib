#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../include/myclib.h"

/* - DEFINITIONS - */

/*
 *   `height`   - The current height of a stack.
 *  `capacity`  - The maximum number of values a stack can store before
 *                expansion is necessary.
 */
typedef struct {
  size_t height;
  size_t capacity;
} stack_header;

#define stack(type) type *

#define STK_EXPANSION_FACTOR ((size_t)2)

/* - CONVENIENCE MACROS - */

#define stack_header(stk) ((stack_header *)(stk) - 1)

#define const_stack_header(stk) ((const stack_header *)(stk) - 1)

#define stack_capacity(stk) (+const_stack_header(stk)->capacity)

#define stack_copy(stk)                                            \
  ((void *)(1 + (stack_header *)memcpy(                            \
                    malloc((sizeof *(stk) * stack_capacity(stk)) + \
                           sizeof(stack_header)),                  \
                    stack_header(stk),                             \
                    (sizeof *(stk) * stack_capacity(stk)) +        \
                        sizeof(stack_header))))

#define stack_copy_s(stk) stack_untyped_copy((void *)(stk), sizeof *(stk))

#define stack_delete(stk) free(stack_header(stk))

#define stack_expand(stk)                    \
  stack_resize(stk, stack_capacity(stk) == 0 \
                        ? 1                  \
                        : STK_EXPANSION_FACTOR * stack_capacity(stk))

#define stack_expand_s(stk) stack_untyped_expand((void **)&(stk), sizeof *(stk))

#define stack_height(stk) (+const_stack_header(stk)->height)

#define stack_is_full(stk) (stack_height(stk) == stack_capacity(stk))

#define stack_is_full_s(stk) stack_untyped_is_full((void *)(stk))

#define stack_is_empty(stk) (stack_height(stk) == 0)

#define stack_new(type, capacity) \
  ((type *)stack_untyped_new(capacity, sizeof(type)))

#define stack_peek(stk) \
  (util_assert(!stack_is_empty(stk)), (stk)[stack_height(stk) - 1])

#define stack_peek_s(stk) stack_untyped_peek((void *)(stk), sizeof *(stk))

#define stack_pop(stk) \
  (util_assert(!stack_is_empty(stk)), (stk)[--stack_header(stk)->height])

#define stack_pop_s(stk) stack_untyped_pop((void *)(stk), sizeof *(stk))

#define stack_push(stk, value)                             \
  (inline_if(stack_is_full(stk), stack_expand(stk), NULL), \
   (stk)[stack_header(stk)->height++] = (value))

#define stack_push_s(stk, value) \
  stack_untyped_push((void **)&(stk), &(value), sizeof *(stk))

#define stack_resize(stk, new_capacity)                                        \
  ((stk) =                                                                     \
       (void *)(1 + (stack_header *)realloc(stack_header(stk),                 \
                                            (sizeof *(stk) * (new_capacity)) + \
                                                sizeof(stack_header))),        \
   util_assert((stk) != NULL),                                                 \
   inline_if(stack_height(stk) > (new_capacity),                               \
             stack_header(stk)->height = (new_capacity), NULL),                \
   stack_header(stk)->capacity = (new_capacity), (stk))

#define stack_resize_s(stk, new_capacity) \
  stack_untyped_resize((void **)&(stk), (size_t)(new_capacity), sizeof *(stk))

#define stack_shrink(stk) stack_resize(stk, stack_height(stk))

#define stack_shrink_s(stk) stack_untyped_shrink((void **)&(stk), sizeof *(stk))

/* - FUNCTIONS - */

stack(void) stack_untyped_copy(const stack(void), size_t value_size);

stack(void) stack_untyped_expand(stack(void) *, size_t value_size);

bool stack_untyped_is_full(const stack(void));

stack(void) stack_untyped_new(size_t capacity, size_t value_size);

void *stack_untyped_peek(stack(void), size_t value_size);

void *stack_untyped_pop(stack(void), size_t value_size);

void *stack_untyped_push(stack(void) *, const void *value, size_t value_size);

stack(void)
    stack_untyped_resize(stack(void) *, size_t new_capacity, size_t value_size);

stack(void) stack_untyped_shrink(stack(void) *, size_t value_size);

#endif
