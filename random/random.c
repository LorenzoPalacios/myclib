#include "random.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

/* Set to `true` for random generator functions to re-seed at a set interval. */
#define AUTO_RESEED_ALLOWED (true)
#if (AUTO_RESEED_ALLOWED)
/*
 * After this many calls to random generator functions, the next call will
 * re-seed.
 */
#define RESEED_INTERVAL (0x100)
static size_t num_calls = 0;
#endif

/*
 * Set to `true` for certain random generator functions to generate
 * a cache of data as needed and return elements from that cache.
 */
#define CACHE_ALLOWED (true)
#if (CACHE_ALLOWED)
/*
 * This determines the size of the cache in bytes.
 * The initial value determines the cache size at startup.
 */
static size_t cache_size;
static void *cache = NULL;
#endif

void init_cache(const size_t initial_capacity) {
  if (cache != NULL) {
    cache = malloc(initial_capacity);
    if (cache == NULL) {
      puts("");
    }
    cache_size = initial_capacity;
  }
}

bool random_bool(void) {
#if (CACHE_ALLOWED)
  return false;
#else
  return rand() & 1;
#endif
}

#include <stdio.h>
int main(void) {
  printf("d");
  return 0;
}
