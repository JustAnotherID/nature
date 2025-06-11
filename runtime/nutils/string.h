#ifndef NATURE_STRING_H
#define NATURE_STRING_H

#include "utils/type.h"

n_string_t *string_new(void *raw_string, int64_t length);

n_string_t *string_concat(n_string_t *a, n_string_t *b);

n_bool_t string_ee(n_string_t *a, n_string_t *b);

n_bool_t string_ne(n_string_t *a, n_string_t *b);

n_bool_t string_lt(n_string_t *a, n_string_t *b);

n_bool_t string_le(n_string_t *a, n_string_t *b);

n_bool_t string_gt(n_string_t *a, n_string_t *b);

n_bool_t string_ge(n_string_t *a, n_string_t *b);

n_int_t rt_string_length(n_string_t *a);

#endif //NATURE_STRING_H
