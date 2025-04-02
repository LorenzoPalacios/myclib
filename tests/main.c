#include <stddef.h>
#include <stdio.h>

#include "../include/compat.h"
#include "framework.h"

/* - DEFINITIONS - */

typedef enum MAIN_MENU_STATUS {
  RUN_TESTS = 0x0,
  RUN_TESTS_NO_FAIL = 0x1,
  CONFIG_TESTS = 0x2
} MAIN_MENU_STATUS;

static const char *const MAIN_MENU_OPTIONS[] = {
    "Run Tests",
    "Run Tests (ignore test failure)",
    "Configure Tests",
};

#define NUM_MAIN_MENU_OPTIONS \
  (sizeof(MAIN_MENU_OPTIONS) / sizeof *(MAIN_MENU_OPTIONS))

/* - FUNCTIONS - */

static void print_list(const char *const *strings, const size_t STR_CNT) {
  size_t i;
  for (i = 0; i < STR_CNT; i++) printf("%zu. %s\n", i, strings[i]);
  fflush(stdout);
}

static inline void display_menu(void) {
  puts("\n - Testing Menu -");
  print_list(MAIN_MENU_OPTIONS, NUM_MAIN_MENU_OPTIONS);
  /* `print_list()` calls `fflush()`. */
}

static inline void warn_unrecognized_input(void) {
  puts("Unrecognized input.");
  fflush(stdout);
}

static inline void warn_index(void) { puts("Invalid index."); fflush(stdout); }

/* - OPTION GETTERS - */

static void get_test_option(test_suite *const suite) {
  const size_t NUM_TESTS = suite->num_tests;
  display_tests(suite);
  display_legend();
  while (true) {
    size_t option;
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_KEYWORD_EXIT:
        return;
      case STATUS_SKIP_ALL:
        skip_all_tests(suite);
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
        warn_unrecognized_input();
        break;
    }
  }
}

static void get_suite_option(void) {
  display_suites();
  display_legend();
  while (true) {
    size_t option;
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_KEYWORD_EXIT:
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
      default:
        warn_unrecognized_input();
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
      /* TBD */
      break;
    case CONFIG_TESTS:
      get_suite_option();
      break;
    default:
      warn_unrecognized_input();
      break;
  }
}

static void main_menu_loop(void) {
  size_t option;
  display_menu();
  while (true) {
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_KEYWORD_EXIT:
        return;
      case STATUS_INDEX:
        parse_main_menu_option(option);
        break;
      default:
        warn_unrecognized_input();
    }
    display_menu();
  }
}

int main(void) {
  (void)setvbuf(stdout, NULL, _IOFBF, BUFSIZ);
  main_menu_loop();
  return 0;
}
