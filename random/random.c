#include "random.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/myclib.h"

/* - DEFINITIONS - */

#define DEFAULT_CACHE_SIZE (4096)

#define cache(type) type *

#define cache_handle(type) cache(type) *

typedef cache(void) * (*const cache_ctor)(void);

typedef void (*const cache_dtor)(void);

static bool caching_enabled = true;

static size_t cache_size = DEFAULT_CACHE_SIZE;

/* Calculated by random_init(). */
static size_t rand_highest_bit;

/* Calculated by random_init(). */
static size_t rand_usable_bytes;

/* - FUNCTION DEFINITIONS - */
static void destroy_cache(cache_handle(void) handle);
static void randomize_bytes(byte *bytes, size_t byte_cnt);

/* - UTILITY - */

static inline cache(void) create_cache(const size_t elem_size) {
  cache(void) cache = calloc(cache_size, elem_size);
  if (cache != NULL) randomize_bytes(cache, cache_size * elem_size);
  return cache;
}

static inline size_t highest_bit_index(register wb_uint n) {
  size_t i = 0;
  while (n >= UINT_MAX) {
    n >>= CHAR_BIT * sizeof(unsigned int) - 1;
    i += CHAR_BIT * sizeof(unsigned int) - 1;
  }
  while (n >= USHRT_MAX) {
    n >>= CHAR_BIT * sizeof(unsigned short);
    i += CHAR_BIT * sizeof(unsigned short);
  }
  while (n >= UCHAR_MAX) {
    n >>= CHAR_BIT;
    i += CHAR_BIT;
  }
  while (n >>= 1) {
    i++;
  }
  return i;
}

static inline void randomize_bytes(byte *const bytes, const size_t byte_cnt) {
  size_t i;
  int val = rand();
  for (i = 0; i < byte_cnt; i++) {
    memset(bytes + i, val, rand_usable_bytes);
    val = rand();
  }
}

/* - CACHE MANIPULATION - */

static inline cache_handle(void) bool_cache_ctor(void) {
  static cache(void) cache = NULL;
  if (cache == NULL) cache = create_cache(sizeof(bool));
  return &cache;
}

static inline bool bool_cache_pop(void) {
  cache_handle(void) handle = bool_cache_ctor();
  bool value;
  if (*handle != NULL) {
    cache(byte) cache = *handle;
    static size_t i = 0;
    if (i == CHAR_BIT * cache_size) {
      i = 0;
      randomize_bytes(cache, cache_size);
    }
    value = (cache[i / CHAR_BIT]) & (1 << (i % CHAR_BIT));
    i++;
  } else {
    value = rand() & 1;
  }
  return value;
}

static inline cache_handle(void) int_cache_ctor(void) {
  static cache(void) cache = NULL;
  if (cache == NULL) cache = create_cache(sizeof(int));
  return &cache;
}

static inline int int_cache_pop(void) {
  cache_handle(void) handle = int_cache_ctor();
  int value;
  if (*handle != NULL) {
    cache(int) cache = *handle;
    static size_t i = 0;
    if (i == cache_size) {
      i = 0;
      randomize_bytes((byte *)cache, cache_size * sizeof(int));
    }
    value = cache[i++];
  } else {
    value = rand();
  }
  return value;
}

static cache_ctor CACHE_CTORS[] = {bool_cache_ctor};

static inline void construct_all_caches(void) {
  size_t i;
  for (i = 0; i < ARR_LEN(CACHE_CTORS); i++) CACHE_CTORS[i]();
}

static inline void destroy_cache(cache_handle(void) handle) {
  if (*handle != NULL) {
    free(*handle);
    *handle = NULL;
  }
}

static inline void destroy_all_caches(void) {
  size_t i;
  for (i = 0; i < ARR_LEN(CACHE_CTORS); i++) destroy_cache(CACHE_CTORS[i]());
}

/* - IMPLEMENTATION - */

bool random_bool(void) {
  return caching_enabled ? bool_cache_pop() : rand() & 1;
}

int random_int(void) {
  int value;
  if (caching_enabled) {
    value = int_cache_pop();
  } else {
    value = 0;
    randomize_bytes((byte *)&value, sizeof(value));
  }
  return value;
}

seed_t random_init(const seed_t *const seed) {
  const seed_t SEED_ACTUAL = (seed != NULL) ? *seed : random_seed();
  srand(SEED_ACTUAL);
  rand_highest_bit = highest_bit_index(RAND_MAX);
  rand_usable_bytes = rand_highest_bit / CHAR_BIT;
  if (caching_enabled) construct_all_caches();
  return SEED_ACTUAL;
}

seed_t random_seed(void) {
  seed_t seed;
  const wb_uint PROGRAM_TIME = (wb_uint)clock();
  /* Not guaranteed to be an integral type, but is often defined as such. */
  const wb_uint EPOCH_TIME = (wb_uint)time(NULL);
#if (IS_STDC11)
  struct timespec t_spec;
  timespec_get(&t_spec, TIME_UTC);
  const wb_uint UTC_NANOSEC = (wb_uint)t_spec.tv_nsec;
  seed = INTEGRAL_CAST((UTC_NANOSEC), seed_t);
#endif
  seed = (seed_t)(PROGRAM_TIME ^ EPOCH_TIME);
  return ~seed;
}
