#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/myclib.h"
#include "framework.h"

/* - DEFINITIONS - */

/* Keywords should be entirely lowercase. */
static const char *const KEYWORDS[] = {
    "all", "skip", "unskip"
};

#define NUM_KEYWORDS (sizeof(KEYWORDS) / sizeof *(KEYWORDS))

typedef enum keyword_status {
  NO_KEYWORD = 0,
  RUN_ALL = 1,
} keyword_status;

#define INT_TO_CHAR(c) ((char)((unsigned char)(c)))

/*
 * This should be enough to hold a string representation of `SIZE_MAX`,
 * any defined keywords, and a null terminator.
 */
#define BUFFER_SIZE (64)

/* - HELPER FUNCTIONS - */

static inline void str_lower(char *str) {
  for (; *str != '\0'; str++) *str = INT_TO_CHAR(tolower(*str));
}

static inline keyword_status is_keyword(const char *str) {
  keyword_status i = 0;
  for (; i < NUM_KEYWORDS; i++) {
    const char *keyword = KEYWORDS[i];
    while (*str == *keyword)
      if (*(str++) == '\0' && *(keyword++) == '\0') return i + 1;
  }
  return NO_KEYWORD;
}

static inline void discard_line(void) { while (getchar() != '\n'); }

/*
 * @returns `NO_KEYWORD`
 */
static keyword_status get_input(size_t *const index_output) {
  size_t i = 0;
  int chr = getchar();
  char buf[BUFFER_SIZE];
  for (; i < sizeof(buf) && chr != '\n' && chr != EOF; i++) {
    buf[i] = INT_TO_CHAR(chr);
    chr = getchar();
  }
  buf[i] = '\0';
  str_lower(buf);
  if (chr != '\n') discard_line();

  if (sscanf(buf, "%zu", index_output) == 0) *index_output = SIZE_MAX;
  return is_keyword(buf);
}

/* - DRIVER FUNCTIONS - */

/* This is intended to be used in tandem with a display function. */
static inline void selection_prompt(void) {
  puts(
      "Select from the following list:\n"
      "0. Return");
}

static inline void prompt_legend(void) {
  puts(
      "- Legend -\n"
      "(*) - An active item.\n"
      "(x) - A skipped item.");
}

static const test *select_tests(const test_suite *const suite) {}

static const test_suite *select_test_suite(void) {
  test_suite *const selection = test_suite_new(NUM_TEST_SUITES);
  puts("- Test Suite List -");
  display_all_suites();
  while (true) {
    size_t index;
    const keyword_status keyword = get_input(&index);
    if (keyword == NO_KEYWORD) {
      if (index == 0) {
        free(selection);
        return NULL;
      }
      if (index < NUM_TEST_SUITES) TEST_SUITES + index;
    }
  }
  return selection;
}

static inline void display_menu(void) {
  puts(
"- myclib Tests -\n"
      "0. run tests\n"
      "1. run tests (ignore test failure)\n"
      "2. configure tests\n"
      "3. exit");
}

int main(void) {
  display_menu();

  return 0;
}
