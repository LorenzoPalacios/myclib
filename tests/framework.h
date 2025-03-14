#ifndef TEST_FRAMEWORK
#define TEST_FRAMEWORK

#include <stddef.h>
#include <time.h>

#include "../include/myclib.h"

/* - DEFINITIONS - */

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

extern const size_t NUM_TEST_SUITES;

/* - FUNCTIONS - */

void display_all_suites(void);

void display_all_tests(void);

void display_suite(const test_suite *suite);

void display_test(const test *test);

void display_tests_in_suite(const test_suite *suite);

void run_test(test *test);

void skip_test(test *test);

test_suite *test_suite_new(size_t test_capacity);

void toggle_skip(test *test);

void unskip_test(test *test);
#endif
