#ifndef TEST_FRAMEWORK
#define TEST_FRAMEWORK

#include <stddef.h>
#include <time.h>

#include "../include/myclib.h"

#define TEST_CASE_ASSERT(expr) \
  if (!(expr)) return false

/* - DEFINITIONS - */

typedef enum legend { LEGEND_SKIPPED = 'x', LEGEND_UNSKIPPED = '*' } legend;

typedef enum input_status {
  /* - Singleton Flags - */
  STATUS_NONE = 0,
  STATUS_ALL = 1,
  STATUS_EXIT = STATUS_ALL << 1,
  STATUS_SKIP = STATUS_EXIT << 1,
  STATUS_RUN = STATUS_SKIP << 1,
  STATUS_HELP = STATUS_RUN << 1,

  STATUS_INDEX = STATUS_HELP << 1,

  /* - Amalgamate Flags - */
  STATUS_SKIP_ALL = STATUS_SKIP | STATUS_ALL,
  STATUS_SKIP_INDEX = STATUS_SKIP | STATUS_INDEX,
  STATUS_RUN_INDEX = STATUS_RUN | STATUS_INDEX
} input_status;

typedef bool (*test_func_t)(void);

typedef struct test {
  const char *const NAME;
  const test_func_t TEST;
  clock_t elapsed;
  bool passed;
  bool skip;
} test;

typedef struct test_suite {
  const char *const NAME;
  test *tests;
  size_t num_tests;
  bool skip;
} test_suite;

typedef void (*test_op)(test *, void *);
typedef void (*test_op_no_arg)(test *);

typedef void (*suite_op)(test_suite *, void *);
typedef void (*suite_op_no_arg)(test_suite *);

extern test_suite test_suites[];

extern const size_t NUM_TEST_SUITES;

/* - FUNCTIONS - */

/* - DISPLAY - */

void display_legend(void);

void display_main_menu(void);

void display_suites(void);

void display_suite_tests(const test_suite *suite);

/* - UTILITY -*/

void for_each_suite(suite_op op, void *args);

void for_each_suite_no_arg(suite_op_no_arg op);

void for_each_test_no_arg(const test_suite *suite, test_op_no_arg op);

void for_each_test(const test_suite *suite, test_op, void *args);

input_status parse_input(size_t *index_output);

/* - CONFIGURATION - */

bool load_config(void);

bool load_config_verbose(void);

bool save_config(void);

bool save_config_verbose(void);

void skip_all_suites(void);

void skip_tests_in_suite(test_suite *suite);

void skip_suite(test_suite *suite);

void skip_test(test *test);

/* - TEST RESULTS - */

bool save_results(void);

bool save_results_verbose(void);

/* - TEST RUNNERS - */

void run_all_suites(void);

void run_suite(test_suite *suite);

void run_suite_ignore_fail(test_suite *suite);

void run_test(test *test);

void run_test_ignore_fail(test *test);

/* - PROMPTS - */

void main_menu_prompt(void);

void test_prompt(test_suite *suite);

void suite_prompt(void);

#endif
