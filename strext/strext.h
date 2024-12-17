#ifndef STR_EXT
#define STR_EXT

#include <stdio.h>

#define BASE_STR_CAPACITY (8192)

/*
 * The factor by which functions will expand a string's allocated memory by.
 * Should be greater than 1.
 */
#define STR_EXPANSION_FACTOR (2)

typedef struct {
  char *data;
  size_t length;
  size_t allocation;
} string;

/* clang-format off */
#define new_string(arg)                   \
  (_Generic((arg),                        \
  const char *: string_from_chars,        \
  char *: string_from_chars,              \
  FILE *: string_from_stream,             \
  signed long long: string_of_capacity,   \
  signed long: string_of_capacity,        \
  signed int: string_of_capacity,         \
  signed short: string_of_capacity,       \
  signed char: string_of_capacity,        \
  unsigned long long: string_of_capacity, \
  unsigned long: string_of_capacity,      \
  unsigned int: string_of_capacity,       \
  unsigned short: string_of_capacity,     \
  unsigned char: string_of_capacity       \
  )(arg))
/* clang-format on */

#define delete_string(str) _delete_string(&(str))
#define delete_string_s(str) _delete_string_s(&(str))

/*
 * Appends `appended` to the end of `dst`, expanding if necessary.
 *
 * \return A pointer associated with the data of `dst`, or `NULL` if
 * the operation failed.
 */
string_t *append_char(string_t *dst, char appended);

/*
 * Appends `src` to the end of `dst_str`, expanding if necessary.
 *
 * \return A pointer associated with the data of `dst`, or `NULL` if the
 * operation failed.
 */
string_t *append_str(string_t *dst, const string_t *src);

/*
 * Appends `src` to the end of `dst_str`, expanding if necessary.
 *
 * \return A pointer associated with the data of `dst`, or `NULL` if the
 * operation failed.
 */
string_t *append_raw_str(string_t *dst, const char *src, size_t src_len);

void _delete_string(string_t **str_obj);

void _delete_string_s(string_t **str_obj);

string_t *erase_string_contents(string_t *const str);

/*
 * Expands the passed string's allocated boundaries by `expansion_factor`,
 * updating the stats of `str_obj` as necessary.
 *
 * \return A (possibly new) pointer associated with the data of `str_obj`,
 * or `NULL` if reallocation failed.
 *
 * \note If reallocation failed, `str_obj` will be unmodified.
 */
string_t *expand_string(string_t *str_obj);

/*
 * Finds the first occurrence of `needle` within `haystack` starting from the
 * beginning of `haystack->data` and replaces it with `replacement`, expanding
 * `haystack` as necessary.
 *
 * \return A (possibly new) pointer associated with the data of `haystack`, or
 * `NULL` if the operation failed.
 *
 * \note A returned `NULL` pointer does not guarantee `haystack` is unmodified
 * by this function.
 */
string_t *find_replace(string_t *haystack, const string_t *needle,
                       const string_t *replacer);

/*
 * Finds all occurrences of `needle` within `haystack` starting from the
 * beginning of `haystack->data`, replacing any findings with `replacement` and
 * expanding `haystack` as necessary.
 *
 * \return A (possibly new) pointer associated with the data of `haystack`, or
 * `NULL` if the operation failed.
 *
 * \note A returned `NULL` pointer does not guarantee `haystack` is unmodified
 * by this function.
 *
 * \attention Function not yet complete.
 */
string_t *find_replace_all(string_t *haystack, const string_t *needle,
                           const string_t *replacement);

/*
 * Reallocates the memory used for `str_obj` to fit `new_size` bytes, updating
 * the stats of `str_obj` as necessary.
 *
 * This function only modifies the memory allocated for characters, meaning
 * there will always be enough space for the data members of `string_t`
 * regardless of the value passed as `new_size`.
 *
 * \return A (possibly new) pointer associated with the data of `str_obj`, or
 * `NULL` if reallocation failed.
 *
 * \note If reallocation failed, `str_obj` will be unmodified.
 */
string_t *resize_string(string_t *str_obj, size_t new_size);

/*
 * Shrinks the memory used for `str_obj` to fit the number of characters it
 * contains.
 *
 * \return A (possibly new) pointer associated with the data of `str_obj`, or
 * `NULL` if reallocation failed.
 */
string_t *shrink_alloc_to_length(string_t *str_obj);

/*
 * Generates a `string_t` object whose `data` consists of the passed raw
 * null-terminated string, `raw_text`.
 *
 * \return A `string_t` object containing characters from `raw_text`, or `NULL`
 * upon failure.
 */
string_t *string_from_chars(const char *raw_text);

/*
 * Generates a `string_t` object whose `data` consists of a single line of
 * characters from `stdin`.
 *
 * \return A `string_t` object containing characters from `stdin`, or `NULL`
 * upon failure.
 */
string_t *string_from_line_stdin(void);

/*
 * Creates a `string_t` object whose `data` consists of characters within
 * `stream` until `EOF` is met.
 *
 * \return A pointer to a `string_t` object containing characters from `stream`,
 * or `NULL` upon failure.
 */
string_t *string_from_stream(FILE *stream);

/*
 * Creates a `string_t` object whose `data` consists of characters from `stream`
 * until either `EOF` or the specified `delim` character is met.
 *
 * \return A pointer to a `string_t` object containing characters from `stream`,
 * or `NULL` upon failure.
 */
string_t *string_from_stream_given_delim(FILE *stream, char delim);

/*
 * Creates a `string_t` object capable of storing `capacity` number of
 * characters.
 *
 * \return A pointer to a `string_t` object of size
 * `capacity + sizeof(string_t)`, or `NULL` upon failure.
 */
string_t *string_of_capacity(const size_t capacity);

#endif
