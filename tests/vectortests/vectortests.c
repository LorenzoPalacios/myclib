#include "vectortests.h"

#include <stddef.h>

#include "../../vector/vector.h"
#include "../framework.h"

#define TEST_DATA_LEN (sizeof(TEST_DATA) / sizeof *(TEST_DATA))

static const int TEST_DATA[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

bool test_vector_clear(void) {
  vector(int) vec = vector_new(int, 3);

  vector_push(vec, 3);
  vector_push(vec, 6);
  vector_push(vec, 9);
  {
    vector_clear(vec);

    vector_set_s(vec, TEST_DATA[1], 5);

    TEST_CASE_ASSERT(vector_get(vec, 0) == 0);
    TEST_CASE_ASSERT(vector_get(vec, 1) == 0);
    TEST_CASE_ASSERT(vector_get(vec, 2) == 0);
  }
  {
    vector_clear_s(vec);

    vector_set_s(vec, TEST_DATA[1], 10);

    TEST_CASE_ASSERT(vector_get(vec, 0) == 0);
    TEST_CASE_ASSERT(vector_get(vec, 1) == 0);
    TEST_CASE_ASSERT(vector_get(vec, 2) == 0);
    TEST_CASE_ASSERT(vector_get(vec, 5) == 0);
  }

  vector_delete(vec);

  return true;
}

bool test_vector_copy(void) {
  vector(vector(int)) vec_holder = vector_new(vector(int), 5);

  vector(int) base = vector_push(vec_holder, vector_new(int, 3));
  const int ELEM = vector_push(base, 2);

  const size_t BASE_LENGTH = vector_length(base);
  const size_t BASE_CAPACITY = vector_capacity(base);

  size_t i;
  for (i = 0; i < TEST_DATA_LEN; i++) {
    {
      const vector(int) vec = vector_push(vec_holder, vector_copy(base));

      TEST_CASE_ASSERT(vector_length(vec) == BASE_LENGTH);
      TEST_CASE_ASSERT(vector_capacity(vec) == BASE_CAPACITY);
      TEST_CASE_ASSERT(vector_get(vec, 0) == ELEM);
    }
    {
      const vector(int) vec = vector_push(vec_holder, vector_copy_s(base));

      TEST_CASE_ASSERT(vector_length(vec) == BASE_LENGTH);
      TEST_CASE_ASSERT(vector_capacity(vec) == BASE_CAPACITY);
      TEST_CASE_ASSERT(vector_get(vec, 0) == ELEM);
    }
  }

  for (i = 0; i < vector_length(vec_holder); i++) {
    vector(int) vec = vector_get(vec_holder, i);
    vector_delete(vec);
  }

  return true;
}

bool test_vector_delete(void) {
  vector(int) vec_1 = vector_new(int, 0);
  vector(int) vec_2 = vector_new(int, 0);

  vector_push(vec_1, 3);
  vector_push(vec_2, vector_get(vec_1, 0));
  vector_delete(vec_1);
  vector_delete_s(vec_2);

  TEST_CASE_ASSERT(vec_1 == NULL);
  TEST_CASE_ASSERT(vec_2 == NULL);

  return true;
}

bool test_vector_expand(void) {
  vector(int) vec = vector_new(int, 0);
  {
    const size_t length = vector_length(vec);
    vector_expand(vec);
    TEST_CASE_ASSERT(vector_length(vec) > length);
  }
  {
    const size_t length = vector_length(vec);
    vector_expand_s(vec);
    TEST_CASE_ASSERT(vector_length(vec) > length);
  }

  vector_delete(vec);

  return true;
}

bool test_vector_get(void) {
  const size_t CAPACITY = 5;
  vector(int) vec = vector_new(int, CAPACITY);

  size_t i;
  for (i = 0; i < TEST_DATA_LEN; i++) vector_push(vec, TEST_DATA[i]);
  for (i = 0; i < vector_length(vec); i++) {
    TEST_CASE_ASSERT(vector_get(vec, i) == TEST_DATA[i]);
    TEST_CASE_ASSERT(*(int *)vector_get_s(vec, i) == TEST_DATA[i]);
  }

  vector_delete(vec);

  return true;
}

bool test_vector_new(void) {
  const size_t CAPACITY = 3;
  vector(int) vec = vector_new(int, CAPACITY);

  TEST_CASE_ASSERT(vector_capacity(vec) == CAPACITY);
  TEST_CASE_ASSERT(vector_length(vec) == 0);

  vector_delete(vec);
  return true;
}

bool test_vector_insert(void) { return true; }

bool test_vector_push(void) {
  const size_t CAPACITY = 3;
  vector(int) vec = vector_new(int, CAPACITY);

  size_t i;
  for (i = 0; i < CAPACITY; i++) {
    const int ELEM = TEST_DATA[i];
    TEST_CASE_ASSERT(vector_push(vec, ELEM) == ELEM);
    TEST_CASE_ASSERT(*(int *)vector_push_s(vec, ELEM) == ELEM);
  }

  return true;
}

bool test_vector_reset(void) {
  vector(int) vec = vector_new(int, 3);

  vector_push(vec, 2);
  TEST_CASE_ASSERT(vector_length(vec) != 0);

  vector_reset(vec);
  TEST_CASE_ASSERT(vector_length(vec) == 0);

  return true;
}

bool test_vector_resize(void) {
  const size_t SMALL = 1 << 5;
  const size_t MEDIUM = 1 << 10;
  const size_t LARGE = 1 << 20;
  vector(int) vec = vector_new(int, 0);

  TEST_CASE_ASSERT(vector_resize(vec, SMALL) != NULL);
  TEST_CASE_ASSERT(vector_length(vec) == SMALL);

  TEST_CASE_ASSERT(vector_resize_s(vec, MEDIUM) != NULL);
  TEST_CASE_ASSERT(vector_length(vec) == MEDIUM);

  TEST_CASE_ASSERT(vector_resize(vec, LARGE) != NULL);
  TEST_CASE_ASSERT(vector_length(vec) == LARGE);

  vector_delete(vec);

  return true;
}

bool test_vector_set(void) {
  vector(int) vec = vector_new(int, 5);
  const size_t POS_1 = 0;
  const size_t POS_2 = 2;
  const size_t POS_3 = vector_capacity(vec) + 4;

  const int ELEM = TEST_DATA[0];

  vector_resize(vec, vector_capacity(vec));

  vector_set(vec, ELEM, POS_1);
  vector_set(vec, ELEM, POS_2);
  vector_set_s(vec, ELEM, POS_3);

  TEST_CASE_ASSERT(vector_get(vec, POS_1) == ELEM);
  TEST_CASE_ASSERT(vector_get(vec, POS_2) == ELEM);
  TEST_CASE_ASSERT(vector_get(vec, POS_3) == ELEM);
  TEST_CASE_ASSERT(vector_get(vec, POS_3 - 1) == 0);

  return true;
}

bool test_vector_shrink(void) {
  vector(int) vec = vector_new(int, 430);

  vector_push(vec, 2);
  vector_shrink(vec);

  TEST_CASE_ASSERT(vector_capacity(vec) == vector_length(vec));

  vector_reset(vec);
  vector_shrink_s(vec);

  TEST_CASE_ASSERT(vector_capacity(vec) == 0);

  vector_delete(vec);

  return true;
}
