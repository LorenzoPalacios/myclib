#ifndef MYCLIB_TESTS
#define MYCLIB_TESTS

#include <stdbool.h>
#include <time.h>

#define RUN_ALL_TESTS_KEYWORD ("ALL")

typedef clock_t (*test)(void);

typedef struct {
  test func;
  clock_t time_taken;
} test_entry;
#endif
