#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../array/array.h"
#include "../random/random.h"
#include "../stack/stack.h"
#include "../strext/strext.h"
#include "../vector/vector.h"

// - DEFINITIONS -

#define RUN_ALL_TESTS_KEYWORD "all"  // Should be lowercase.

typedef void (*test)(void);

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

#define INT_TO_CHAR(c) ((char)((unsigned char)(c)))

// This should be enough to hold a string representation of `SIZE_MAX`,
// `RUN_ALL_TESTS_KEYWORD`, and a null terminator.
#define BUFFER_SIZE (64)

#define TEST_DATA "abc123!@#\n\t"
#define TEST_DATA_LEN (ARR_LEN(TEST_DATA))
#define TEST_DATA_STR_LEN (TEST_DATA_LEN - 1)

// - GENERIC TEST OPERATIONS -

// For any feature that defines a for-each operation.
static bool for_each_operation(void *const elem) {
  assert(elem != NULL);
  return true;
}

// - TEST FUNCTIONS -

static void test_array(void) {
  array arr = array_new(TEST_DATA);
  assert(arr.length == ARR_LEN(TEST_DATA));
  assert(arr.elem_size == sizeof *(TEST_DATA));

  for (size_t i = 0; i < arr.length; i++) {
    const char *const elem = array_get(arr, i);
    assert(elem != NULL);
    assert(*elem == TEST_DATA[i]);
  }
  assert(array_get(arr, arr.length) == NULL);

  array_for_each(arr, for_each_operation);

  array_clear(arr);
  assert(*(char *)array_get(arr, 0) == 0);

  array_delete(arr);
}

static void test_random(void) {
  static const size_t ITERATIONS = 1000000;
  {
    const seed_t SEED = random_init();
    printf("test_random: SEED = %u\n", SEED);
  }

  size_t cnt_true = 0;
  for (size_t i = 0; i < ITERATIONS; i++) cnt_true += random_bool();

  const double PERCENT_TRUE = (double)cnt_true / (double)ITERATIONS;
  printf("test_random: %g%% true over %zu iterations.\n", PERCENT_TRUE,
         ITERATIONS);
}

static void test_strext(void) {
  string *str = string_new(TEST_DATA);
  assert(str != NULL);
  assert(strcmp(str->data, TEST_DATA) == 0);
  assert(str->length == TEST_DATA_STR_LEN);

  string_clear(str);
  assert(str->length == 0);
  assert(str->data[0] == 0);
  {
    str = string_append_raw_str(str, TEST_DATA);
    assert(str != NULL);
    str = string_append_str(str, str);
    assert(str != NULL);

    assert(str->length == 2 * TEST_DATA_STR_LEN);
    assert(strncmp(str->data, TEST_DATA, TEST_DATA_STR_LEN) == 0);

    char *const str_data_pos = str->data + TEST_DATA_STR_LEN;
    assert(strncmp(str_data_pos, TEST_DATA, TEST_DATA_STR_LEN) == 0);
  }
  str = string_append_int(str, -1);
  assert(str != NULL);

  assert(str->length == 2 * TEST_DATA_STR_LEN + 2);
  assert(str->data[str->length - 2] == '-');
  assert(str->data[str->length - 1] == '1');
  {
    str = string_find_replace(str, "-1", str);
    assert(str != NULL);
    assert(str->length == 4 * TEST_DATA_STR_LEN + 2);
    char *const str_data_pos = str->data + (2 * TEST_DATA_STR_LEN);
    assert(strncmp(str_data_pos, TEST_DATA, TEST_DATA_STR_LEN) == 0);
  }

  string_delete(str);
}

static void test_vector(void) {
  {
    const vector vec = vector_new(TEST_DATA);
    assert(vec.length == ARR_LEN(TEST_DATA));
    for (size_t i = 0; i < vec.length; i++) {
      const char *const elem = vector_get(vec, i);
      assert(elem != NULL);
      assert(*elem == TEST_DATA[i]);
    }
    assert(vector_get(vec, vec.length) == NULL);
    vector_delete(vec);
  }
  {
    vector vec = vector_init(size_t, 10);
    for (size_t i = 0; i < ARR_LEN(TEST_DATA); i++) vector_insert(vec, &i, i);

    assert(vec.length == ARR_LEN(TEST_DATA));
    vector_for_each(vec, for_each_operation);

    vector_delete(vec);
  }
}

// - INTERNAL -

#define TESTS ((test[]){test_array, test_random, test_strext, test_vector})
#define TEST_NAMES \
  ((const char *const[]){"array", "random", "strext", "vector"})

#define NUM_TESTS (sizeof(TESTS) / sizeof(*TESTS))
#define NUM_TEST_NAMES (sizeof(TEST_NAMES) / sizeof(*TEST_NAMES))

static_assert(NUM_TESTS == NUM_TEST_NAMES,
              "Number of tests and test names must match.");
#undef NUM_TEST_NAMES

typedef struct {
  size_t num_selected;
  size_t test_indicies[NUM_TESTS];
} test_selection;

static inline void prompt_user(void) {
  puts("Your test choices are:");
  for (size_t i = 0; i < NUM_TESTS; i++) printf("%zu. %s\n", i, TEST_NAMES[i]);
  puts(
      "\nEnter the number(s) corresponding to the tests you want to run, or "
      "\"" RUN_ALL_TESTS_KEYWORD
      "\" to run all tests.\n"
      "Press the enter/return key to complete your selection and begin running "
      "your selected tests.");
}

static void str_lower(char *str) {
  while (*str != '\0') {
    *str = INT_TO_CHAR(tolower(*str));
    str++;
  }
}

// Helper function used by `get_test_selection`.
// `str` should be capable of holding `str_cap + 1` characters and a null
// terminator.
static void get_line_from_stdin(char *const str, const size_t str_cap) {
  for (size_t i = 0; i < str_cap; i++) {
    const int chr = getchar();
    if (chr == '\n' || chr == EOF) {
      str[i] = '\0';
      str_lower(str);
      return;
    }
    str[i] = INT_TO_CHAR(chr);
  }
  str[str_cap] = '\0';
  while (getchar() != '\n');  // Discard any unused input.
}

static test_selection get_test_selection(void) {
  test_selection selection_obj;
  size_t *const test_indicies = selection_obj.test_indicies;

  selection_obj.num_selected = 0;
  for (size_t i = 0; i < NUM_TESTS; i++) {
    char buf[BUFFER_SIZE];
    get_line_from_stdin(buf, sizeof(buf) - 1);

    // A single null terminator can only be present if either a newline was
    // reached, or `stdin` is in `EOF` state. In either case, we don't need to
    // worry about input anymore.
    if (buf[0] == '\0') break;

    if (strcmp(RUN_ALL_TESTS_KEYWORD, buf) == 0) {
      for (size_t j = 0; j < NUM_TESTS; j++) test_indicies[j] = j;
      selection_obj.num_selected = NUM_TESTS;
      break;
    }

    size_t test_index = NUM_TESTS;
    (void)sscanf(buf, "%zu", &test_index);
    if (test_index < NUM_TESTS) {
      test_indicies[i] = test_index;
      selection_obj.num_selected++;
    } else {  // Redo iteration on invalid entry.
      i--;
    }
  }
  return selection_obj;
}

static void run_tests(const test_selection *const test_selection) {
  clock_t total_elapsed = 0;

  const size_t *const test_indicies = test_selection->test_indicies;
  const size_t test_cnt = test_selection->num_selected;
  for (size_t i = 0; i < test_cnt; i++) {
    const size_t CUR_TEST_INDEX = test_indicies[i];

    printf("Beginning Test %zu (%s)...\n", i, TEST_NAMES[CUR_TEST_INDEX]);

    const clock_t TEST_START = clock();
    TESTS[CUR_TEST_INDEX]();
    const clock_t TEST_ELAPSED = clock() - TEST_START;

    const double ELAPSED_SEC = (double)TEST_ELAPSED / CLOCKS_PER_SEC;
    printf("Test %zu (%s) took %g seconds.\n\n", i, TEST_NAMES[CUR_TEST_INDEX],
           ELAPSED_SEC);
    total_elapsed += TEST_ELAPSED;
  }

  const double total_elapsed_sec = (double)total_elapsed / CLOCKS_PER_SEC;
  const double avg_elapsed_sec = total_elapsed_sec / (double)test_cnt;
  printf("Total elapsed time: %g seconds.\n", total_elapsed_sec);
  printf("Average test run time: %g seconds.\n", avg_elapsed_sec);
}

int main(void) {
  prompt_user();
  const test_selection selection = get_test_selection();
  run_tests(&selection);
  return 0;
}
