#include <stddef.h>
#include <stdio.h>

#include "../include/compat.h"
#include "framework.h"

/* - DEFINITIONS - */

typedef enum MAIN_MENU_STATUS {
  RUN_TESTS = 0x0,
  RUN_TESTS_NO_FAIL = 0x1,
  CONFIG_TESTS = 0x2,
  EXIT = 0x3
} MAIN_MENU_STATUS;

#define BUFFER_SIZE (128)

static const char *const MAIN_MENU_OPTIONS[] = {
    "Run Tests",
    "Run Tests (ignore test failure)",
    "Configure Tests",
    "Exit",
};

#define NUM_MAIN_MENU_OPTIONS \
  (sizeof(MAIN_MENU_OPTIONS) / sizeof *(MAIN_MENU_OPTIONS))

/* - FUNCTIONS - */

static inline size_t count_digits(size_t num) {
  size_t i = 0;
  while (i++, num /= 10);
  return i;
}

static inline void display_legend(void) {
  puts(
      "- Legend -\n"
      "(*) - An active item.\n"
      "(x) - A skipped item.");
}

static void print_buffered(const char *const *strings, const size_t STR_CNT) {
  char buf[BUFFER_SIZE];
  size_t buf_i = 0;
  size_t str_i = 0;
  for (; str_i < STR_CNT; str_i++) {
    size_t chr_i = 0;
    char chr;
    if (buf_i + count_digits(str_i) + 2 >= sizeof(buf) - 1) {
      (void)fputs(buf, stdout);
      buf_i = 0;
    }
    buf_i += (size_t)sprintf(buf + buf_i, "%zu. ", str_i);
    do {
      chr = strings[str_i][chr_i];
      if (buf_i == sizeof(buf) - 1) {
        (void)fputs(buf, stdout);
        buf_i = 0;
      }
      buf[buf_i] = chr;
      buf_i++, chr_i++;
    } while (chr != '\0');
    buf[buf_i - 1] = '\n';
    buf[buf_i] = '\0';
  }
  (void)fputs(buf, stdout);
}

static inline void display_menu(void) {
  puts(" - Testing Menu -");
  print_buffered(MAIN_MENU_OPTIONS, NUM_MAIN_MENU_OPTIONS);
}

static inline void parse_main_menu_option(const size_t option) {
  switch (option) {
    case RUN_TESTS: {
      run_all_suites();
      break;
    }
    case RUN_TESTS_NO_FAIL: {
      break;
    }
    case CONFIG_TESTS: {
      puts("\n- Suites -");
      display_suites();
      display_legend();
      break;
    }
    default:
      break;
  }
}

static size_t get_menu_option(void) {
  size_t option;
  while (get_input(&option) != STATUS_INDEX ||
         option > NUM_MAIN_MENU_OPTIONS - 1) {
    puts("Unrecognized input.");
  }
  return option;
}

int main(void) {
  display_menu();
  {
    const size_t option = get_menu_option();
    parse_main_menu_option(option);
  }

  return 0;
}
