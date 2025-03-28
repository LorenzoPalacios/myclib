#include "stacktests.h"

#include <assert.h>
#include <stddef.h>

#include "../../stack/stack.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

void test_stack_copy(void) {
  const int arr[] = {1, 2, 3, 4, 5};
  stack(stack(int)) stk1 = stack_init(stack(int), 3);
  stack(stack(int)) stk1_copy;

  stack_push_s(stk1, stack_new(arr));
  stack_push_s(stk1, stack_new(arr));
  stack_push_s(stk1, stack_new(arr));

  stk1_copy = stack_copy(stk1);

  assert(stack_capacity(stk1) == stack_capacity(stk1_copy));
  assert(stack_height(stk1) == stack_height(stk1_copy));
  assert(stack_height(stk1) == ARR_LEN(arr));

  while (!stack_is_empty(stk1)) {
    stack(int) int_stk = *stack_pop(stk1);
    stack(int) int_stk_copy = *stack_pop(stk1_copy);
    size_t i = ARR_LEN(arr) - 1;

    /*
     * This is a pointer comparison; `stack_copy` does not perform a deep
     * copy.
     */
    assert(int_stk == int_stk_copy);

    for (; !stack_is_empty(int_stk); i++) {
      const int int_original_val = arr[i];
      const int int_stk_val = *stack_pop(int_stk);
      const int int_stk_copy_val = *stack_pop(int_stk_copy);
      assert(int_original_val == int_stk_val);
      assert(int_stk_val == int_stk_copy_val);
    }
  }

  stack_delete(stk1);
  stack_delete(stk1_copy);
}

void test_stack_delete(void) {
  stack(int) stk1 = stack_init(int, 3);
  stack(int) stk2 = stack_copy(stk1);

  assert(stk1 != NULL);
  stack_delete(stk1);
  assert(stk1 == NULL);
  stack_delete_s(stk2);
  assert(stk2 == NULL);
}

void test_stack_expand(void) {
  const size_t INITIAL_CAPACITY = 0;
  stack(int) stk1 = stack_init(int, INITIAL_CAPACITY);
  stack(int) stk2 = stack_copy(stk1);

  stack_expand(stk1);
  assert(stack_capacity(stk1) > INITIAL_CAPACITY);
  stack_expand(stk2);
  assert(stack_capacity(stk2) > INITIAL_CAPACITY);

  stack_delete(stk1);
  stack_delete(stk2);
}

void test_stack_init(void) {
  const size_t CAPACITY = 3;
  stack(int) stk = stack_init(int, CAPACITY);

  assert(stack_capacity(stk) == CAPACITY);
  assert(stack_peek(stk) == NULL);
  assert(stack_is_empty(stk));

  stack_delete(stk);
}

void test_stack_new(void) {
  const int arr[] = {1, 2, 3};
  stack(int) stk = stack_new(arr);

  size_t i = ARR_LEN(arr) - 1;
  for (; !stack_is_empty(stk); i++) {
    const int stk_value = *stack_pop(stk);
    assert(stk_value == arr[i]);
  }

  stack_delete(stk);
}

void test_stack_peek(void) {
  const int arr[] = {1, 2, 3};
  stack(int) stk1 = stack_new(arr);
  stack(int) stk2 = stack_init(int, 1);

  assert(*stack_peek(stk1) == arr[ARR_LEN(arr) - 1]);
  assert(stack_peek_s(stk2) == NULL);

  stack_delete(stk1);
  stack_delete(stk2);
}

void test_stack_pop(void) {
  const int arr[] = {1, 2, 3};
  stack(int) stk1 = stack_new(arr);
  stack(int) stk2 = stack_init(int, 1);

  const int stk1_value = *stack_pop(stk1);
  const int *const stk2_value = stack_pop_s(stk2);
  assert(stk1_value == arr[ARR_LEN(arr) - 1]);
  assert(stk2_value == NULL);

  stack_delete(stk1);
  stack_delete(stk2);
}

void test_stack_push(void) {
  const int arr[] = {1, 2, 3};
  stack(int) stk = stack_init(int, 0);
  const size_t INITIAL_HEIGHT = stack_height(stk);

  {
    size_t i = 0;
    for (; i < ARR_LEN(arr); i++) stack_push(stk, arr + i);
  }

  assert(stack_height(stk) != INITIAL_HEIGHT);

  {
    size_t i = 0;
    while (i < ARR_LEN(arr)) stack_push_s(stk, arr + i++);
  }

  while (!stack_is_empty(stk)) {
    size_t i = 0;
    for (; i < ARR_LEN(arr); i++) {
      const int stk_value = *stack_pop(stk);
      assert(stk_value == arr[i]);
    }
  }

  stack_delete(stk);
}

void test_stack_reset(void) {
  const int arr[] = {1, 2, 3};
  stack(int) stk1 = stack_new(arr);

  assert(!stack_is_empty(stk1));
  stack_reset(stk1);
  assert(stack_is_empty(stk1));
  stack_delete(stk1);
}

void test_stack_resize(void) {
  const size_t INITIAL_CAPACITY = 3;
  const size_t NEW_CAPACITY = 4;
  stack(int) stk = stack_init(int, INITIAL_CAPACITY);

  stack_resize(stk, NEW_CAPACITY);
  assert(stack_capacity(stk) == NEW_CAPACITY);
  {
    int i = 0;
    for (; (size_t)i < NEW_CAPACITY; i++) assert(stack_push_s(stk, &i));
  }

  stack_delete(stk);
}

void test_stack_shrink(void) {
  const size_t CAPACITY = 3;
  stack(int) stk = stack_init(int, CAPACITY);

  stack_shrink(stk);
  assert(stack_capacity(stk) == 0);

  stack_expand(stk);

  stack_shrink_s(stk);
  assert(stack_capacity(stk) == 0);

  stack_delete(stk);
}
