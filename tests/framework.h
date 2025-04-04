#ifndef TEST_FRAMEWORK
#define TEST_FRAMEWORK

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "../include/boolmyclib.h"

#define TEST_SUITE_ASSERT(expr) \
  if (!(expr)) return false

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
  clock_t elapsed;
  bool passed;
  bool skip;
} test;

typedef struct test_suite {
  const char *const name;
  test *tests;
  size_t num_tests;
  bool skip;
} test_suite;

typedef void (*test_op)(test *, void *);
typedef void (*test_op_no_arg)(test *);

typedef void (*suite_op)(test_suite *, void *);
typedef void (*suite_op_no_arg)(test_suite *);

extern test_suite TEST_SUITES[];

extern const size_t NUM_TEST_SUITES;

#define CONFIG_FILENAME "test.config"

#define RESULTS_FILENAME "test.results"

/* - FUNCTIONS - */

/* - DISPLAY -*/
void display_legend(void);

void display_suites(void);

void display_tests(const test_suite *suite);

/* - UTILITY -*/

void for_each_suite(suite_op op, void *args);

void for_each_suite_no_arg(suite_op_no_arg op);

void for_each_test_no_arg(test_suite *suite, test_op_no_arg op);

void for_each_test(test_suite *suite, test_op, void *args);

input_status parse_input(size_t *index_output);

void run_all_suites(void);

void run_all_suites_ignore_fail(void);

void run_suite(test_suite *suite);

void run_suite_ignore_fail(test_suite *suite);

void run_test(test *test);

void run_test_ignore_fail(test *test);

void skip_all_suites(void);

void skip_all_tests(test_suite *suite);

void skip_suite(test_suite *suite);

void skip_test(test *test);

bool save_test_results(void);

/* - CONFIGURATION - */

bool load_test_config(void);

bool save_test_config(void);

#endif
