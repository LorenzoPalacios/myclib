#include "random.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if (CACHE_ALLOWED)

typedef void *cache;
typedef cache *(*cache_constructor)(void);

#define INT_CACHE_SIZE (CACHED_ELEMS * sizeof(int))
// We use `INT_CACHE_SIZE` because `rand()` returns an int.
#define BOOL_CACHE_SIZE (INT_CACHE_SIZE)

// - CACHE STOCKERS -

/**
 * @brief Stocks the integer cache with random values.
 *
 * This function fills the provided cache with random integer values.
 *
 * @param cache Pointer to the cache to be stocked.
 */
static void stock_int_cache(void *const cache) {
  for (size_t i = 0; i < CACHED_ELEMS; i++) {
    ((int *)cache)[i] = rand();
  }
}

/**
 * @brief Stocks the boolean cache with random values.
 *
 * This function fills the provided cache with random boolean values.
 *
 * @param cache Pointer to the cache to be stocked.
 */
static inline void stock_bool_cache(void *const cache) {
  stock_int_cache(cache);
}

// - CACHE CONSTRUCTORS -

/**
 * @brief Retrieves the integer cache.
 *
 * This function returns a pointer to the integer cache, allocating and
 * stocking it if necessary.
 *
 * @return Pointer to the integer cache.
 */
static cache *get_int_cache(void) {
  static cache int_cache = NULL;
  if (int_cache == NULL) {
    int_cache = malloc(INT_CACHE_SIZE);
    if (int_cache != NULL) {
      stock_int_cache(int_cache);
    }
  }
  return &int_cache;
}

/**
 * @brief Retrieves the boolean cache.
 *
 * This function returns a pointer to the boolean cache, allocating and
 * stocking it if necessary.
 *
 * @return Pointer to the boolean cache.
 */
static cache *get_bool_cache(void) {
  static cache bool_cache = NULL;
  if (bool_cache == NULL) {
    bool_cache = malloc(BOOL_CACHE_SIZE);
    if (bool_cache != NULL) {
      stock_bool_cache(bool_cache);
    }
  }
  return &bool_cache;
}

// - INTERNAL IMPLEMENTATION -

/**
 * @brief Finds the highest bit index in RAND_MAX.
 *
 * This function calculates and returns the highest bit index in RAND_MAX.
 *
 * @return The highest bit index in RAND_MAX.
 */
static inline size_t rand_max_highest_bit_index(void) {
  static size_t HIGHEST_BIT_INDEX = 0;
  if (HIGHEST_BIT_INDEX == 0) {
    // Although `RAND_MAX` is guaranteed to be at least `32767` and at most
    // `INT_MAX`, two's complement is not guaranteed, so we cannot initialize
    // `bit` with `15`.
    size_t bit = 0;
    while ((RAND_MAX >> bit) != 0) {
      bit++;
    }
    HIGHEST_BIT_INDEX = bit;
  }
  return HIGHEST_BIT_INDEX;
}

/**
 * @brief Retrieves a random boolean value from the cache.
 *
 * This function returns a random boolean value from the cache, restocking
 * the cache as necessary.
 *
 * @note This method restocks its cache after
 * `(CACHED_ELEMS * RAND_MAX_HIGHEST_BIT_INDEX)` hits.
 *
 * @return A random boolean value.
 */
static bool get_bool_from_cache(void) {
  int *const cache = *get_bool_cache();

  if (cache == NULL) {
    return rand() & 1;
  }

  static size_t cache_index = 0;
  static size_t bit_index = 0;

  if (cache_index == CACHED_ELEMS) {
    bit_index++;
    cache_index = 0;
    /*
     * The highest bit is unlikely to show up with `rand()`, so we reset
     * `bit_index` despite the highest bit technically being valid. If we did
     * not reset `bit_index`, a majority of the returned values will be `false`,
     * since many of the cache elements will not have this bit set by due to
     * being less than `RAND_MAX`.
     */
    if (bit_index == rand_max_highest_bit_index()) {
      bit_index = 0;
      stock_bool_cache(cache);
    }
  }
  return cache[cache_index++] & (1 << bit_index);
}

/**
 * @brief Retrieves a random integer value from the cache.
 *
 * This function returns a random integer value from the cache, restocking
 * the cache as necessary.
 *
 * @return A random integer value.
 */
static int get_int_from_cache(void) {
  int *const cache = *get_int_cache();

  if (cache == NULL) {
    return rand();
  }

  static size_t cache_index = 0;

  if (cache_index == CACHED_ELEMS) {
    cache_index = 0;
    stock_int_cache(cache);
  }
  return cache[cache_index++];
}

// Cache constructors go here
#define cache_constructors \
  ((cache_constructor[]){get_bool_cache, get_int_cache})
#define NUM_GETTERS (sizeof(cache_constructors) / sizeof *(cache_constructors))

// - PUBLIC CACHE-RELATED FUNCTIONS -

void random_destroy_caches(void) {
  for (size_t i = 0; i < NUM_GETTERS; i++) {
    cache *const cache = cache_constructors[i]();
    free(*cache);
    *cache = NULL;
  }
}

seed_t random_init(void) {
  seed_t SEED;
  {
    struct timespec t_spec;
    timespec_get(&t_spec, TIME_UTC);
    const long long UTC_NS = t_spec.tv_nsec;
    const long long EPOCH_TIME = time(NULL);
    const long long CLOCK_TIME = clock();
    SEED = (seed_t)(UTC_NS ^ EPOCH_TIME ^ CLOCK_TIME);
  }
  srand(SEED);

  // The first call to the cache constructors will allocate and stock the
  // caches.
  for (size_t i = 0; i < NUM_GETTERS; i++) {
    void *const cache = *cache_constructors[i]();
    if (cache == NULL) {
      fprintf(stderr, "random_init(): Failed to create cache %zu\n", i);
    }
  }
  // The first call to this function finds the highest bit in `RAND_MAX`, which
  // will be returned by any subsequent calls.
  rand_max_highest_bit_index();
  return SEED;
}
#endif

// - RANDOM GENERATORS -

bool random_bool(void) {
#if (CACHE_ALLOWED)
  return get_bool_from_cache();
#else
  return rand() & 1;
#endif
}

// This function will need revising to deal with the variable range of `rand()`.
int random_int(void) {
#if (CACHE_ALLOWED)
  return get_int_from_cache();
#else
  return rand();
#endif
}
