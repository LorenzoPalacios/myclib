#include "tests.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../array/array.h"
#include "../random/random.h"
#include "../stack/stack.h"
#include "../strext/strext.h"
#include "../vector/vector.h"

// - TEST FUNCTIONS -

static clock_t test_array_new(void) {
  const int data[] = {1, 2, 3, 4, 5};
  array *const arr = new_array(data);
  if (arr == NULL) return 0;

  for (size_t i = 0; i < arr->length; i++) {
    const int *const elem = array_get(arr, i);
    if (elem == NULL || *elem != data[i]) {
      array_delete(arr);
      return 0;
    }
  }

  array_delete(arr);
  return clock();
}

#define TESTS ((test){_test_new_array})
static const char *const test_names[NUM_TESTS] = {"new_array()"};

static void prompt_user(void) {
  puts("Your test choices are:");
  for (size_t i = 0; i < NUM_TESTS; i++) printf("%zu. %s\n", i, test_names[i]);
  printf(
      "\nEnter the number(s) of the tests you want to run, or \"%s\" to run "
      "all tests.\n"
      "Press the enter/return key to complete your selection and begin running "
      "the selected tests.\n",
      RUN_ALL_TESTS_KEYWORD);
}

/*
 * Helper function used by `get_test_selection`.
 * `fgets()` retains newlines inside input, thereby making detection of
 * `RUN_ALL_TESTS_KEYWORD` difficult, and `scanf()` is too finicky.
 */
static void get_line_from_stdin(char *const str, const size_t max_len) {
  for (size_t i = 0; i < max_len; i++) {
    const char chr = getchar();
    if (chr == '\n') {
      str[i] = '\0';
      return;
    }
    str[i] = chr;
  }
  // Discard any unused input.
  while (getchar() != '\n');
}

/*
 * Asks the user to choose from a list of available tests as defined by
 * `tests` and `test_names`.
 *
 * \returns An array of function pointers where each element corresponds to a
 * given test function.
 *
 * \note As it stands, this function was written with the intent of getting it
 * out of the way so the rest of this test suite can be worked on.
 * Bearing this in mind, the quality is not up to my usual standards, and I
 * intend to refactor this function later.
 */
static test_entry *get_test_selection(void) {
  test_entry *const selected_tests =
      malloc(sizeof(test_entry) * (NUM_TESTS + 1));

  char buf[DIGITS_IN_NUM(SIZE_MAX) + 1];
  size_t test_index;
  size_t i = 0;
  for (; i < NUM_TESTS; i++) {
    get_line_from_stdin(buf, sizeof(buf) - 1);
    str_to_lower(buf);
    if (strcmp(RUN_ALL_TESTS_KEYWORD, buf) == 0) {
      for (size_t j = 0; j < NUM_TESTS; j++) {
        selected_tests[i].func = TESTS[i];
        selected_tests[i].time_taken = 0;
      }
      selected_tests[NUM_TESTS].func = NULL;
      return selected_tests;
    }

    const int sscanf_status = sscanf(buf, "%zu", &test_index);
    if (sscanf_status == 1 && test_index < NUM_TESTS) {
      selected_tests[i].func = TESTS[test_index];
      selected_tests[i].time_taken = 0;
    } else if (sscanf_status == EOF) { /* Exit on single newline. */
      break;
    } else { /* Redo iteration on invalid entry. */
      i--;
    }
  }
  selected_tests[i].func = NULL; /* Terminator element for `selected_tests`. */
  return selected_tests;
}

static clock_t run_tests(test_entry *const test_selection) {
  clock_t total_time = 0;
  for (size_t i = 0; test_selection[i].func != NULL; i++) {
    const clock_t time_taken = test_selection[i].func();
    test_selection[i].time_taken = time_taken;
    total_time += time_taken;
  }
  return total_time;
}

int main(void) {
  greet_and_init();
  prompt_user();
  test_entry *const selected_tests = get_test_selection();
  const size_t total_time_taken = run_tests(selected_tests);
  free(selected_tests);
  printf("Total elapsed time: %zu ms\n", total_time_taken);

  return 0;
}
