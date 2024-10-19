#include "random.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

unsigned char *random_unsigned_raw_string(const unsigned char min,
                                          const unsigned char max,
                                          const size_t length) {
  /* (length + 1) to account for the null terminator. */
  unsigned char *str = malloc(length + 1);
  if (str == NULL) return NULL;

  for (size_t i = 0; i < length; i++)
    str[i] = random_unsigned_char_in_range(min, max);

  str[length] = '\0';

  return str;
}

/* Returns a random string of `char` using `random_vis_uchar()`. */
char *random_raw_string(const char min, const char max, const size_t length) {
  // (length + 1) to account for the null terminator
  char *str = malloc(length + 1);
  if (str == NULL) return NULL;

  for (size_t i = 0; i < length; i++)
    str[i] = random_unsigned_char_in_range(min, max);
  str[length] = '\0';

  return str;
}

char *random_alphabetical_raw_string(const size_t length) {
  /* (length + 1) to account for the null terminator */
  char *str = malloc(length + 1);
  if (str == NULL) return NULL;

  for (size_t i = 0; i < length; i++) {
    if (random_bool() & 1)
      str[i] = random_unsigned_char_in_range('a', 'z');
    else
      str[i] = random_unsigned_char_in_range('A', 'Z');
  }
  str[length] = '\0';

  return str;
}

char random_visible_char(void) {
#if (!ALLOW_RANDOM_GEN_CACHING)
  return random_int(VIS_CHAR_START, CHAR_MAX);
#else
  static char cache[CACHE_SIZE];
  static size_t iterator = CACHE_SIZE;

  if (iterator == CACHE_SIZE) {
    for (size_t i = 0; i < CACHE_SIZE; i++)
      cache[i] = random_int(VIS_CHAR_START, CHAR_MAX);
    iterator = 0;
  }
  return cache[iterator++];
#endif
}

unsigned char random_unsigned_char_in_range(const unsigned char min,
                                            const unsigned char max) {
  return random_int(min, max);
}

unsigned char random_visible_unsigned_char(void) {
#if (!ALLOW_RANDOM_GEN_CACHING)
  return random_int(VIS_CHAR_START, UCHAR_MAX);
#else
  static unsigned char cache[CACHE_SIZE];
  static size_t iterator = CACHE_SIZE;

  if (iterator == CACHE_SIZE) {
    for (size_t i = 0; i < CACHE_SIZE; i++)
      cache[i] = random_int(VIS_CHAR_START, UCHAR_MAX);
    iterator = 0;
  }
  return cache[iterator++];
#endif
}

bool random_bool(void) {
#if (!ALLOW_RANDOM_GEN_CACHING)
  return rand() & 1;
#else
  /*
   * For this particular cache, the stored elements in the below array are used
   * as bitfields. The bits within each bitfield are accessed by shifting `1`
   * to the left `bit_iterator` times. Once `bit_iterator` reaches the maximum
   * number of bits within that bitfield, `cache_iterator` is incremented so
   * the next bitfield can be used, and `bit_iterator` is reset to 0.
   */
  static unsigned char cache[CACHE_SIZE];
  /*
   * Setting the iterator to the end of the cache will prompt its restocking
   * upon the initial call to this function.
   */
  static size_t cache_iterator = CACHE_SIZE;
  static unsigned char bit_iterator = 1;
  if (bit_iterator == CHAR_BIT * sizeof(*cache)) {
    bit_iterator = 1;
    cache_iterator++;
  }

  if (cache_iterator == CACHE_SIZE) {
    for (size_t i = 0; i < CACHE_SIZE; ++i) {
      cache[i] = rand();
    }
    cache_iterator = 0;
  }
  return cache[cache_iterator] & (1 << bit_iterator++);
#endif
}

int random_int(const int min, const int max) {
  return rand() % (max - min) + min;
}
