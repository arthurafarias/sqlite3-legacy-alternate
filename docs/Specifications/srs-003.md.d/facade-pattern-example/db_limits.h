#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct db_limits db_limits;
struct db_limits {
  int max_key_len;
  int max_value_len;
  int max_rows;
};

extern const db_limits db_limits_default;

#ifdef __cplusplus
}
#endif
