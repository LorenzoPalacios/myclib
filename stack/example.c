/*
 * Define this in above `#include "stack.h"` to expose some functionality
 * allowing the use of stacks that do not rely on heap allocations.
 */
#define STACK_INCL_HEAPLESS_STACK
#include <stdio.h>

#include "stack.h"

int main(void) {
  /*
   * Example usage of `_new_heapless_interface_stack()`.
   *
   * This snippet should output:
   *
   * `5 4 3 2 1`
   */
  {
    int data[] = {1, 2, 3, 4, 5};
    stack interface_stk = _heapless_new_interface_stack(
        data, sizeof(data) / sizeof(*data), sizeof(*data));

    int *val;
    while ((val = interface_stack_pop(&interface_stk))) {
      printf("%d ", *val);
    }

    putchar('\n');
    /*
     * Deletion is unnecessary since `interface_stk` was not allocated on the
     * heap.
     */
  }

  /*
   * Example usage of `_new_stack()`.
   *
   * This snippet should output:
   *
   * `1 2 3 4 5 `
   */
  {
    const int data[] = {1, 2, 3, 4, 5};
    const size_t DATA_LEN = sizeof(data) / sizeof(*data);
    stack *stk_from_arr = _new_stack(data, DATA_LEN, sizeof(*data));
    if (stk_from_arr == NULL) return 1;

    /* Printing and popping all of the values in `stk_from_arr`. */

    int *val = stack_pop(stk_from_arr);
    while (val != NULL) {
      printf("%d ", *val);
      val = stack_pop(stk_from_arr);
    }

    delete_stack(&stk_from_arr);
    putchar('\n');
  }

  /*
   * Example usage of `clear_stack()`.
   *
   * This snippet should output:
   *
   * `1 (N/A: val is NULL)`
   */
  {
    const int data[] = {1, 2, 3, 4, 5};
    stack *example_stk = new_stack(data);

    /* Print the top value of `example_stk`. */
    int *val = stack_peek(example_stk);
    if (val == NULL) return 1;
    printf("%d ", *val);

    clear_stack(example_stk);

    /* Attempt to print the top value of `example_stk` again. */
    val = stack_peek(example_stk);
    if (val == NULL) printf("(N/A: val is NULL)");

    delete_stack(&example_stk);
    putchar('\n');
  }

  /*
   * Example usage of `interface_stack_push()`.
   *
   * This snippet should output:
   *
   * `0 0 0 | 0 0 1 `
   *
   */
  {
    int data[] = {0, 0, 0};
    size_t DATA_LEN = sizeof(data) / sizeof(*data);
    stack interface_stk = heapless_new_interface_stack(data);

    /* Printing the initial contents of `data`. */
    for (size_t i = 0; i < DATA_LEN; i++) {
      printf("%d ", data[i]);
    }

    printf("| ");

    int new_value = 1;
    /* "Removes" the last element in `data`. */
    interface_stack_pop(&interface_stk);
    /* Writes to the last position in `data`.*/
    interface_stack_push(&interface_stk, &new_value);

    for (size_t i = 0; i < DATA_LEN; i++) {
      printf("%d ", data[i]);
    }

    putchar('\n');
    /*
     * Deletion is unnecessary since `interface_stk` was not allocated on the
     * heap.
     */
  }

  /*
   * Example usage of `resize_stack()`.
   *
   * This snippet could output:
   *
   * `4 1600 20 1300 64`
   */
  {
    const size_t ELEM_SIZE = sizeof(int);
    size_t num_elems = 1;
    stack *example_stk = new_empty_stack(num_elems, ELEM_SIZE);
    if (example_stk == NULL) return 1;

    /* Initial capacity. */
    printf("%zu ", example_stk->capacity);

    /* Upscaling by a factor of `example_stk->elem_size`. */
    num_elems = 400;
    example_stk = resize_stack(example_stk, ELEM_SIZE * num_elems);
    if (example_stk == NULL) return 1;
    printf("%zu ", example_stk->capacity);

    /* Downscaling by a factor of `example_stk->elem_size`. */
    num_elems = 5;
    example_stk = resize_stack(example_stk, ELEM_SIZE * num_elems);
    if (example_stk == NULL) return 1;
    printf("%zu ", example_stk->capacity);

    /* Upscaling by a non-factor of `example_stk->elem_size`. */
    num_elems = 100;
    example_stk = resize_stack(example_stk, 13 * num_elems);
    if (example_stk == NULL) return 1;
    printf("%zu ", example_stk->capacity);

    /* Downscaling by a non-factor of `example_stk->elem_size`. */
    num_elems = 9;
    example_stk = resize_stack(example_stk, 7 * num_elems);
    if (example_stk == NULL) return 1;
    printf("%zu", example_stk->capacity);

    delete_stack(&example_stk);
    putchar('\n');
  }

  /*
   * Example usage of `shrink_stack_to_fit()`.
   *
   * This snippet could output:
   *
   * `40 12`
   */
  {
    const size_t MAX_NUM_ELEMS = 10;
    const size_t ELEM_SIZE = sizeof(int);
    stack *example_stk = new_empty_stack(MAX_NUM_ELEMS, ELEM_SIZE);
    if (example_stk == NULL) return 1;

    /* Initial capacity. */
    printf("%zu ", example_stk->capacity);

    const int RANDOM_VAL_1 = 3;
    example_stk = stack_push(example_stk, &RANDOM_VAL_1);
    const int RANDOM_VAL_2 = 932;
    example_stk = stack_push(example_stk, &RANDOM_VAL_2);
    const int RANDOM_VAL_3 = 0xD00D;
    example_stk = stack_push(example_stk, &RANDOM_VAL_3);

    example_stk = shrink_stack_to_fit(example_stk);
    if (example_stk == NULL) return 1;

    printf("%zu", example_stk->capacity);

    delete_stack(&example_stk);
    putchar('\n');
  }

  /*
   * Example usage of `new_empty_stack()`.
   *
   * This snippet should output:
   *
   * `2147483647 1 0 5 4 3 2 1 `
   */
  {
    const int data[] = {1, 2, 3, 4, 5};
    const size_t DATA_LEN = sizeof(data) / sizeof(*data);

    const size_t INITIAL_NUM_ELEMS = 5;
    stack *example_stk = new_empty_stack(INITIAL_NUM_ELEMS, sizeof(*data));
    if (example_stk == NULL) return 1;

    /* Pushing the elements of `data` to `example_stk`. */
    for (size_t i = 0; i < DATA_LEN; i++) {
      example_stk = stack_push(example_stk, data + i);
      if (example_stk == NULL) return 1; /* Reallocation could fail. */
    }

    /* Pushing arbitrary values to `example_stk`. */

    const int RANDOM_VAL_1 = 0;
    example_stk = stack_push(example_stk, &RANDOM_VAL_1);
    if (example_stk == NULL) return 1;

    const int RANDOM_VAL_2 = 1;
    example_stk = stack_push(example_stk, &RANDOM_VAL_2);
    if (example_stk == NULL) return 1;

    /*
     * Since this is the last element to be pushed onto `example_stk`, it
     * will be the first to be popped.
     */
    const int RANDOM_VAL_3 = 0x7FFFFFFF; /* 32-bit `signed int` maximum value */
    example_stk = stack_push(example_stk, &RANDOM_VAL_3);
    if (example_stk == NULL) return 1;

    /* Printing and popping all of the values in `example_stk`. */

    int *val = stack_pop(example_stk);
    while (val != NULL) {
      printf("%d ", *val);
      val = stack_pop(example_stk);
    }

    delete_stack(&example_stk);
    putchar('\n');
  }

  /*
   * Example usage of `new_stack()`, a convenience macro over
   * `_new_stack()`.
   *
   * This snippet should output:
   *
   * `1 2 3 4 5 `
   */
  {
    const int data[] = {1, 2, 3, 4, 5};
    stack *example_stk = new_stack(data);
    if (example_stk == NULL) return 1;

    /* Printing and popping all of the values in `example_stk`. */

    int *val = stack_pop(example_stk);
    while (val != NULL) {
      printf("%d ", *val);
      val = stack_pop(example_stk);
    }

    delete_stack(&example_stk);
    putchar('\n');
  }

  /*
   * Example usage of `heapless_new_empty_stack()`
   *
   * This snippet should output:
   *
   * `4 3 2 1 0 `
   */
  {
    const size_t NUM_ELEMS = 5;
    stack example_stk =
        heapless_new_empty_stack(example_stk, NUM_ELEMS, sizeof(int));

    /* Push values onto the stack until capacity is reached. */
    for (size_t i = 0; i < NUM_ELEMS; i++) {
      heapless_stack_push(&example_stk, &i);
    }

    /*
     * This push should not succeed since the stack is full and cannot be
     * resized.
     */
    const int RANDOM_VAL_1 = -1;
    heapless_stack_push(&example_stk, &RANDOM_VAL_1);

    /* Pop and print all of the values in the stack. */
    int *val;
    while ((val = heapless_stack_pop(&example_stk)) != NULL) {
      printf("%d ", *val);
    }
    putchar('\n');
    /*
     * Deletion is unnecessary since `interface_stk` was not allocated on the
     * heap.
     */
  }
  return 0;
}
