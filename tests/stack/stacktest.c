#include "stacktest.h"

#include <assert.h>
#include <stddef.h>

#include "../../stack/stack.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

void test_stack_new_(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack stk = stack_new_(data, ARR_LEN(data), sizeof(*data));
  for (size_t i = ARR_LEN(data); i > 0; i--)
    assert(*(int *)stack_pop(stk) == data[i]);
  stack_delete(stk);
}

void test_stack_interface_push(void) {
  int data[] = {0, 0, 0};
  size_t DATA_LEN = sizeof(data) / sizeof(*data);
  stack stk = stack_auto_interface_new(data);

  int new_value = 1;
  stack_pop(stk);
  stack_auto_push(&stk, &new_value);

  // Printing the modified contents of `data`.
  for (size_t i = 0; i < DATA_LEN; i++) {
    printf("%d ", data[i]);
  }

  // Deletion is unnecessary since `stk` was not allocated on the
  // heap.
}

/*
 * Example usage of `stack_resize_()`.
 *
 * This snippet could output:
 *
 * `4 1600 20 1300 64`
 */
void test_stack_resize(void) {
  const size_t ELEM_SIZE = sizeof(int);
  const size_t INITIAL_CAPACITY = 1;
  stack stk = stack_empty_new(INITIAL_CAPACITY, ELEM_SIZE);
  assert(stk != NULL);

  // Initial capacity.
  printf("%zu ", stack_capacity(stk));

  const size_t LARGE_CAPACITY = 9999;
  stk = stack_resize_(stk, LARGE_CAPACITY);
  assert(stk != NULL);
  printf("%zu ", stack_capacity(stk));

  const size_t NO_CAPACITY = 0;
  stk = stack_resize_(stk, NO_CAPACITY);
  assert(stk != NULL);
  printf("%zu ", stack_capacity(stk));

  const size_t SMALL_CAPACITY = 5;
  stk = stack_resize_(stk, SMALL_CAPACITY);
  assert(stk != NULL);
  printf("%zu ", stack_capacity(stk));

  const size_t MEDIUM_CAPACITY = 700;
  stk = stack_resize_(stk, MEDIUM_CAPACITY);
  assert(stk != NULL);
  printf("%zu ", stack_capacity(stk));

  stack_delete(stk);
}

/*
 * Example usage of `stack_shrink_to_fit()`.
 *
 * This snippet could output:
 *
 * `10 3`
 */
void test_stack_shrink_to_fit(void) {
  const size_t INITIAL_CAPACITY = 10;
  const size_t ELEM_SIZE = sizeof(int);
  stack stk = stack_empty_new(INITIAL_CAPACITY, ELEM_SIZE);
  assert(stk != NULL);

  // Initial capacity.
  printf("%zu ", stack_capacity(stk));

  const int RANDOM_VAL_1 = 3;
  stk = stack_push(stk, &RANDOM_VAL_1);
  assert(stk != NULL);
  const int RANDOM_VAL_2 = 932;
  stk = stack_push(stk, &RANDOM_VAL_2);
  assert(stk != NULL);
  const int RANDOM_VAL_3 = 0xD00D;
  stk = stack_push(stk, &RANDOM_VAL_3);
  assert(stk != NULL);

  stk = stack_shrink_to_fit(stk);
  assert(stk != NULL);

  printf("%zu", stack_capacity(stk));

  stack_delete(stk);
}

/*
 * Example usage of `stack_empty_new()`.
 *
 * This snippet should output:
 *
 * `2147483647 1 0 5 4 3 2 1 `
 */
void test_stack_empty_new(void) {
  const int data[] = {1, 2, 3, 4, 5};
  const size_t DATA_LEN = sizeof(data) / sizeof(*data);

  const size_t INITIAL_CAPACITY = 5;
  stack stk = stack_empty_new(INITIAL_CAPACITY, sizeof(*data));
  assert(stk != NULL);

  /* Pushing the elements of `data` to `stk`. */
  for (size_t i = 0; i < DATA_LEN; i++) {
    stk = stack_push(stk, data + i);
    assert(stk != NULL);
  }

  // Pushing arbitrary values to `stk`.

  const int RANDOM_VAL_1 = 0;
  stk = stack_push(stk, &RANDOM_VAL_1);
  assert(stk != NULL);
  const int RANDOM_VAL_2 = 1;
  stk = stack_push(stk, &RANDOM_VAL_2);
  assert(stk != NULL);
  // Since this is the last element to be pushed onto `stk`, it
  // will be the first to be popped.
  const int RANDOM_VAL_3 = 0x7FFFFFFF;  // 32-bit `signed int` maximum value
  stk = stack_push(stk, &RANDOM_VAL_3);
  assert(stk != NULL);

  // Pop and print all of the values in the stack.
  int *val = stack_pop_(stk);
  while (val != NULL) {
    printf("%d ", *val);
    val = stack_pop_(stk);
  }

  stack_delete(stk);
}
/*
 * Example usage of `stack_new()`, a convenience macro over
 * `stack_new_()`.
 *
 * This snippet should output:
 *
 * `5 4 3 2 1 `
 */
void test_stack_new(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack stk = stack_new(data);
  assert(stk != NULL);

  // Pop and print all of the values in the stack.
  int *val = stack_pop_(stk);
  while (val != NULL) {
    printf("%d ", *val);
    val = stack_pop_(stk);
  }

  stack_delete(stk);
}

/*
 * Example usage of `stack_auto_empty_new()`
 *
 * This snippet should output:
 *
 * `5 4 3 2 1 `
 */
void test_stack_auto_empty_new(void) {
  const size_t NUM_ELEMS = 5;
  stack stk = stack_auto_empty_new(stk, NUM_ELEMS, sizeof(int));

  /* Push values onto the stack until capacity is reached. */
  for (size_t i = 1; i <= NUM_ELEMS; i++) {
    stack_auto_push(&stk, &i);
  }

  // This push should not succeed since the stack is full and cannot be
  // resized.
  const int RANDOM_VAL_1 = -1;
  stack_auto_push(&stk, &RANDOM_VAL_1);

  // Pop and print all of the values in the stack.
  int *val;
  while ((val = stack_pop_(&stk)) != NULL) {
    printf("%d ", *val);
  }
  // Deletion is unnecessary since `stk` was not allocated on
  // the heap.
}
