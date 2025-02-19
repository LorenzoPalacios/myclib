#ifndef RANDOM_H
#define RANDOM_H

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L)
#if (__STDC_VERSION__ < 202311L)
/* For C99 to C17. */
#include <stdbool.h>
#endif
#else
/* For C95 and below. */
#if (!(defined true || defined false))
typedef unsigned char bool;
#define true (1)
#define false (0)
#endif
#define inline
#endif

typedef unsigned int seed_t;

/*
 * Set to `true` for certain random generator functions to generate
 * a cache of data as needed and return elements from that cache.
 */
#define CACHE_ALLOWED (true)

#if (CACHE_ALLOWED)

/* This is the size of each generator's cache in terms of elements. */
#define CACHED_ELEMS (8192)

/**
 * @brief Destroys all caches used by the random generator functions.
 *
 * This function frees any memory allocated for caches used by the random
 * generator functions.
 */
void random_destroy_caches(void);

#endif

/**
 * @brief Initializes the random generator.
 *
 * This function sets up the random generator with an initial seed, and if
 * `RANDOM_CACHE_ALLOWED` is `true`, will allocate a cache for each generator.
 *
 * This should be called before any random generation functions are used.
 *
 * @return seed_t The initial seed value used for the random number generator.
 */
seed_t random_init(void);

/**
 * @brief Returns a random boolean value.
 *
 * If caching is enabled, the first call to this function will create a
 * cache, then return the first element from the created cache.
 *
 * @return `true` or `false`.
 */
bool random_bool(void);

/**
 * @brief Returns a random integer value.
 *
 * This function generates and returns a random integer value.
 *
 * @return A random integer.
 */
int random_int(void);

#endif
