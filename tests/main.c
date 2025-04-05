#include <stddef.h>
#include <stdio.h>

#include "../include/compat.h"
#include "framework.h"

/* - DEFINITIONS - */

typedef enum MAIN_MENU_STATUS {
  RUN_TESTS = 0,
  RUN_TESTS_NO_FAIL = 1,
  CONFIG_TESTS = 2,
  LOAD_CONFIG = 3,
  SAVE_CONFIG = 4,
  SAVE_RESULTS = 5
} MAIN_MENU_STATUS;

/* - FUNCTIONS - */

/* - OPTION GETTERS - */

static void get_test_option(test_suite *const suite) {
  const size_t NUM_TESTS = suite->num_tests;
  display_tests(suite);
  display_legend();
  while (true) {
    size_t option;
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_EXIT:
        return;
      case STATUS_SKIP_ALL:
        skip_tests_in_suite(suite);
        display_tests(suite);
        display_legend();
        break;
      case STATUS_SKIP_INDEX:
        if (option >= NUM_TESTS) {
          warn_index();
          break;
        }
        skip_test(suite->tests + option);
        display_tests(suite);
        display_legend();
        break;
      default:
        warn_bad_input();
        break;
    }
  }
}

static inline void notify_suite_ran(void) {}

static void get_suite_option(void) {
  display_suites();
  display_legend();
  while (true) {
    size_t option;
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_EXIT:
        return;
      case STATUS_SKIP_ALL:
        skip_all_suites();
        display_suites();
        display_legend();
        break;
      case STATUS_SKIP_INDEX:
        if (option >= NUM_TEST_SUITES) {
          warn_index();
          break;
        }
        skip_suite(TEST_SUITES + option);
        display_suites();
        display_legend();
        break;
      case STATUS_INDEX:
        if (option >= NUM_TEST_SUITES) {
          warn_index();
          break;
        }
        get_test_option(TEST_SUITES + option);
        display_suites();
        display_legend();
        break;
      case STATUS_RUN_INDEX:
        run_suite(TEST_SUITES + option);
        printf("Ran suite %s.", TEST_SUITES[option].name);
        break;
      default:
        warn_bad_input();
        break;
    }
  }
}

/* - OPTION PARSERS - */

static inline void parse_main_menu_option(const size_t option) {
  switch (option) {
    case RUN_TESTS:
      run_all_suites();
      break;
    case RUN_TESTS_NO_FAIL:
      /*run_all_suites_ignore_fail();*/
      break;
    case CONFIG_TESTS:
      get_suite_option();
      break;
    case LOAD_CONFIG:
      load_config_verbose();
      break;
    case SAVE_CONFIG:
      save_config_verbose();
      break;
    case SAVE_RESULTS:
      save_results_verbose();
      break;
    default:
      warn_bad_input();
      break;
  }
}

static void main_menu_loop(void) {
  size_t option;
  display_menu();
  while (true) {
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_EXIT:
        return;
      case STATUS_INDEX:
        parse_main_menu_option(option);
        break;
      default:
        warn_bad_input();
    }
    display_menu();
  }
}

int main(void) {
  (void)setvbuf(stdout, NULL, _IOFBF, BUFSIZ);
  load_config_quiet();
  main_menu_loop();
  return 0;
}
