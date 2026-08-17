#include "db_limits.h"

const db_limits db_limits_default = {
    .max_key_len = 63,
    .max_value_len = 255,
    .max_rows = 16,
};
