#ifndef TEST_FRAMEWORK
#define TEST_FRAMEWORK

#include <stddef.h>
#include <time.h>

#include "../include/boolmyclib.h"

#define TOGGLE_SKIP(skippable) (skippable).skip = !(skippable).skip

#define TEST_SUITE_ASSERT(expr) \
  if (expr) return false

/* - DEFINITIONS - */

typedef enum legend { LEGEND_SKIPPED = 'x', LEGEND_UNSKIPPED = '*' } legend;

typedef enum input_status {
  /* - Single Flags - */
  STATUS_NO_KEYWORD = 0,
  STATUS_KEYWORD_ALL = 1,
  STATUS_KEYWORD_EXIT = STATUS_KEYWORD_ALL << 1,
  STATUS_KEYWORD_SKIP = STATUS_KEYWORD_EXIT << 1,
  STATUS_INDEX = STATUS_KEYWORD_SKIP << 1,
  STATUS_INVALID = STATUS_INDEX << 1,

  /* - Amalgamate Flags - */
  STATUS_SKIP_ALL = STATUS_KEYWORD_SKIP | STATUS_KEYWORD_ALL,
  STATUS_SKIP_INDEX = STATUS_KEYWORD_SKIP | STATUS_INDEX
} input_status;

typedef bool (*test_func_t)(void);

typedef struct test {
  const char *const name;
  const test_func_t test;
  const size_t name_len;
  clock_t elapsed;
  bool passed;
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
void display_legend(void);

void display_suites(void);

void display_tests(const test_suite *suite);

input_status parse_input(size_t *index_output);

void run_all_suites(void);

void run_suite(test_suite *suite);

void run_test(test *test);

void skip_all_suites(void);

void skip_all_tests(test_suite *suite);

void skip_suite(test_suite *suite);

void skip_test(test *test);
#endif
