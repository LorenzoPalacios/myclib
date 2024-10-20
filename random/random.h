/*
 * This file utilizes rand().
 * Ensure srand() is called with a valid seed before using these functions.
 */

#ifndef RANDOM_H
#define RANDOM_H

#include <stdbool.h>

/*
 * Returns either `true` or `false`.
 *
 * If caching is enabled, the first call to this function will create a cache,
 * then return the first element from the created cache.
 */
bool random_bool(void);
#endif
