#include <assert.h>
#include <stdio.h>

#include "stack.h"

/*
 * Example usage of `stack_heapless_interface_new_()`.
 *
 * This snippet should output:
 *
 * `5 4 3 2 1 `
 */
void ex_stack_heapless_interface_new_(void) {
  int data[] = {1, 2, 3, 4, 5};
  stack interface_stk = stack_heapless_interface_new_(
      data, sizeof(data) / sizeof(*data), sizeof(*data));

  // Pop and print all of the values in the stack.
  int *val;
  while ((val = stack_pop(&interface_stk))) {
    printf("%d ", *val);
  }

  (void)putchar('\n');
  // Deletion is unnecessary since `interface_stk` was not allocated on the
  // heap.
}

/*
 * Example usage of `stack_new_()`.
 *
 * This snippet should output:
 *
 * `5 4 3 2 1 `
 */
void ex_stack_new_(void) {
  const int data[] = {1, 2, 3, 4, 5};
  const size_t DATA_LEN = sizeof(data) / sizeof(*data);
  stack *example_stk = stack_new_(data, DATA_LEN, sizeof(*data));
  assert(example_stk != NULL);

  // Pop and print all of the values in the stack.
  int *val = stack_pop(example_stk);
  while (val != NULL) {
    printf("%d ", *val);
    val = stack_pop(example_stk);
  }

  stack_delete(example_stk);
  (void)putchar('\n');
}

/*
 * Example usage of `stack_clear()`.
 *
 * This snippet should output:
 *
 * `5 N/A (val is NULL)`
 */
void ex_stack_clear(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack *example_stk = stack_new(data);
  assert(example_stk != NULL);

  // Print the top value of `example_stk`.
  int *val = stack_peek(example_stk);
  printf("%d ", *val);

  stack_clear(example_stk);

  // Attempt to print the top value of `example_stk` again.
  val = stack_peek(example_stk);
  if (val == NULL) printf("N/A (val is NULL)");

  stack_delete(example_stk);
  (void)putchar('\n');
}

/*
 * Example usage of `stack_interface_push()`.
 *
 * This snippet should output:
 *
 * `0 0 0 | 0 0 1 `
 *
 */
void ex_stack_interface_push(void) {
  int data[] = {0, 0, 0};
  size_t DATA_LEN = sizeof(data) / sizeof(*data);
  stack interface_stk = stack_heapless_interface_new(data);

  // Printing the initial contents of `data`.
  for (size_t i = 0; i < DATA_LEN; i++) {
    printf("%d ", data[i]);
  }

  printf("| ");

  int new_value = 1;
  // "Removes" the last element in `data`.
  stack_pop(&interface_stk);
  // Writes to the last position in `data`.
  stack_heapless_push(&interface_stk, &new_value);

  // Printing the modified contents of `data`.
  for (size_t i = 0; i < DATA_LEN; i++) {
    printf("%d ", data[i]);
  }

  (void)putchar('\n');
  // Deletion is unnecessary since `interface_stk` was not allocated on the
  // heap.
}

/*
 * Example usage of `stack_resize()`.
 *
 * This snippet could output:
 *
 * `4 1600 20 1300 64`
 */
void ex_stack_resize(void) {
  const size_t ELEM_SIZE = sizeof(int);
  const size_t INITIAL_CAPACITY = 1;
  stack *example_stk = stack_empty_new(INITIAL_CAPACITY, ELEM_SIZE);
  assert(example_stk != NULL);

  // Initial capacity.
  printf("%zu ", stack_capacity(example_stk));

  const size_t LARGE_CAPACITY = 9999;
  example_stk = stack_resize(example_stk, LARGE_CAPACITY);
  assert(example_stk != NULL);
  printf("%zu ", stack_capacity(example_stk));

  const size_t NO_CAPACITY = 0;
  example_stk = stack_resize(example_stk, NO_CAPACITY);
  assert(example_stk != NULL);
  printf("%zu ", stack_capacity(example_stk));

  const size_t SMALL_CAPACITY = 5;
  example_stk = stack_resize(example_stk, SMALL_CAPACITY);
  assert(example_stk != NULL);
  printf("%zu ", stack_capacity(example_stk));

  const size_t MEDIUM_CAPACITY = 700;
  example_stk = stack_resize(example_stk, MEDIUM_CAPACITY);
  assert(example_stk != NULL);
  printf("%zu ", stack_capacity(example_stk));

  stack_delete(example_stk);
  (void)putchar('\n');
}

/*
 * Example usage of `stack_shrink_to_fit()`.
 *
 * This snippet could output:
 *
 * `10 3`
 */
void ex_stack_shrink_to_fit(void) {
  const size_t INITIAL_CAPACITY = 10;
  const size_t ELEM_SIZE = sizeof(int);
  stack *example_stk = stack_empty_new(INITIAL_CAPACITY, ELEM_SIZE);
  assert(example_stk != NULL);

  // Initial capacity.
  printf("%zu ", stack_capacity(example_stk));

  const int RANDOM_VAL_1 = 3;
  example_stk = stack_push(example_stk, &RANDOM_VAL_1);
  assert(example_stk != NULL);
  const int RANDOM_VAL_2 = 932;
  example_stk = stack_push(example_stk, &RANDOM_VAL_2);
  assert(example_stk != NULL);
  const int RANDOM_VAL_3 = 0xD00D;
  example_stk = stack_push(example_stk, &RANDOM_VAL_3);
  assert(example_stk != NULL);

  example_stk = stack_shrink_to_fit(example_stk);
  assert(example_stk != NULL);

  printf("%zu", stack_capacity(example_stk));

  stack_delete(example_stk);
  (void)putchar('\n');
}

/*
 * Example usage of `stack_empty_new()`.
 *
 * This snippet should output:
 *
 * `2147483647 1 0 5 4 3 2 1 `
 */
void ex_stack_empty_new(void) {
  const int data[] = {1, 2, 3, 4, 5};
  const size_t DATA_LEN = sizeof(data) / sizeof(*data);

  const size_t INITIAL_CAPACITY = 5;
  stack *example_stk = stack_empty_new(INITIAL_CAPACITY, sizeof(*data));
  assert(example_stk != NULL);

  /* Pushing the elements of `data` to `example_stk`. */
  for (size_t i = 0; i < DATA_LEN; i++) {
    example_stk = stack_push(example_stk, data + i);
    assert(example_stk != NULL);
  }

  // Pushing arbitrary values to `example_stk`.

  const int RANDOM_VAL_1 = 0;
  example_stk = stack_push(example_stk, &RANDOM_VAL_1);
  assert(example_stk != NULL);
  const int RANDOM_VAL_2 = 1;
  example_stk = stack_push(example_stk, &RANDOM_VAL_2);
  assert(example_stk != NULL);
  // Since this is the last element to be pushed onto `example_stk`, it
  // will be the first to be popped.
  const int RANDOM_VAL_3 = 0x7FFFFFFF;  // 32-bit `signed int` maximum value
  example_stk = stack_push(example_stk, &RANDOM_VAL_3);
  assert(example_stk != NULL);

  // Pop and print all of the values in the stack.
  int *val = stack_pop(example_stk);
  while (val != NULL) {
    printf("%d ", *val);
    val = stack_pop(example_stk);
  }

  stack_delete(example_stk);
  (void)putchar('\n');
}
/*
 * Example usage of `stack_new()`, a convenience macro over
 * `stack_new_()`.
 *
 * This snippet should output:
 *
 * `5 4 3 2 1 `
 */
void ex_stack_new(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack *example_stk = stack_new(data);
  assert(example_stk != NULL);

  // Pop and print all of the values in the stack.
  int *val = stack_pop(example_stk);
  while (val != NULL) {
    printf("%d ", *val);
    val = stack_pop(example_stk);
  }

  stack_delete(example_stk);
  (void)putchar('\n');
}

/*
 * Example usage of `stack_heapless_empty_new()`
 *
 * This snippet should output:
 *
 * `5 4 3 2 1 `
 */
void ex_stack_heapless_empty_new(void) {
  const size_t NUM_ELEMS = 5;
  stack example_stk =
      stack_heapless_empty_new(example_stk, NUM_ELEMS, sizeof(int));

  /* Push values onto the stack until capacity is reached. */
  for (size_t i = 1; i <= NUM_ELEMS; i++) {
    stack_heapless_push(&example_stk, &i);
  }

  // This push should not succeed since the stack is full and cannot be
  // resized.
  const int RANDOM_VAL_1 = -1;
  stack_heapless_push(&example_stk, &RANDOM_VAL_1);

  // Pop and print all of the values in the stack.
  int *val;
  while ((val = stack_pop(&example_stk)) != NULL) {
    printf("%d ", *val);
  }
  (void)putchar('\n');
  // Deletion is unnecessary since `interface_stk` was not allocated on
  // the heap.
}

typedef void (*example_func)(void);

#define EXAMPLE_FUNCTIONS               \
  ((example_func[]){                    \
      ex_stack_clear,                   \
      ex_stack_empty_new,               \
      ex_stack_heapless_empty_new,      \
      ex_stack_heapless_interface_new_, \
      ex_stack_new,                     \
      ex_stack_new_,                    \
      ex_stack_resize,                  \
      ex_stack_shrink_to_fit,           \
  })

#define NUM_EXAMPLE_FUNCTIONS \
  (sizeof(EXAMPLE_FUNCTIONS) / sizeof *(EXAMPLE_FUNCTIONS))

int main(void) {
  for (size_t i = 0; i < NUM_EXAMPLE_FUNCTIONS; i++) EXAMPLE_FUNCTIONS[i]();
  return 0;
}
