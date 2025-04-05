#ifndef TEST_FRAMEWORK
#define TEST_FRAMEWORK

#include <stddef.h>
#include <time.h>

#include "../include/boolmyclib.h"

#define TEST_SUITE_ASSERT(expr) \
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

/* - DISPLAY - */

void display_legend(void);

void display_menu(void);

void display_suites(void);

void display_tests(const test_suite *suite);

/* - UTILITY -*/

void for_each_suite(suite_op op, void *args);

void for_each_suite_no_arg(suite_op_no_arg op);

void for_each_test_no_arg(const test_suite *suite, test_op_no_arg op);

void for_each_test(const test_suite *suite, test_op, void *args);

input_status parse_input(size_t *index_output);

/* - FEEDBACK - */

void warn_bad_input(void);

void warn_index(void);

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

void run_all_suites_ignore_fail(void);

void run_suite(const test_suite *suite);

void run_suite_ignore_fail(const test_suite *suite);

void run_test(test *test);

void run_test_ignore_fail(test *test);
#endif
