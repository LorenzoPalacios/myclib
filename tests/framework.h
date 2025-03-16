#ifndef TEST_FRAMEWORK
#define TEST_FRAMEWORK

#include <stddef.h>
#include <time.h>

#include "../include/boolmyclib.h"
#include "../include/compat.h"

/* - DEFINITIONS - */

typedef enum input_status {
  STATUS_KEYWORD_ALL = 0x0,
  STATUS_KEYWORD_EXIT = 0x1,
  STATUS_KEYWORD_SKIP = 0x2,
  STATUS_NO_KEYWORD = 0x3,
  STATUS_BAD_INPUT = 0x10,
  STATUS_INDEX = 0x20
} input_status;

typedef void (*test_func_t)(void);

typedef struct test {
  const char *const name;
  const test_func_t test;
  const size_t name_len;
  clock_t elapsed;
  bool skip;
} test;

typedef struct test_suite {
  const char *const name;
  test *tests;
  const size_t name_len;
  size_t num_tests;
  bool skip;
} test_suite;

extern test_suite TEST_SUITES[];

extern const size_t NUM_KEYWORDS;

extern const size_t NUM_TEST_SUITES;

/* - FUNCTIONS - */

void display_suites(void);

void display_tests(void);

void display_suite(const test_suite *suite);

void display_test(const test *test);

void display_suite_tests(const test_suite *suite);

input_status get_input(size_t *index_output);

void run_all_suites(void);

void run_suite(test_suite *suite);

void run_test(test *test);

void toggle_skip(test *test);
#endif
