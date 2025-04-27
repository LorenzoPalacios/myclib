#ifndef RANDOM_H
#define RANDOM_H

#include "../include/myclib.h"

typedef unsigned int seed_t;

seed_t random_seed(void);

bool random_create_caches(void);

/**
 * @brief Destroys all caches used by the random generator functions.
 *
 * This function frees any memory allocated for caches used by the random
 * generator functions.
 */
bool random_destroy_caches(void);

/**
 * @brief Initializes the random generator.
 *
 * This function sets up the random generator with an initial seed, and if
 * `RANDOM_CACHE_ALLOWED` is `true`, will allocate a cache for each
 * generator.
 *
 * This should be called before any random generation functions are used.
 *
 * @return seed_t The seed value used for the random number generator.
 */
seed_t random_init(const seed_t *seed);

/**
 * @brief Returns a random boolean value.
 * @return `true` or `false`.
 */
bool random_bool(void);

/**
 * @brief Returns a random integer value.
 *
 * @return A random integer.
 */
int random_int(void);

#endif
