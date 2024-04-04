#include "s21_string.h"

s21_size_t s21_strcspn(const char *string, const char *takeOut) {
  s21_size_t count = 0;
  while (*string != '\0') {
    const char *takeOut_ptr = takeOut;
    while (*takeOut_ptr != '\0') {
      if (*string == *takeOut_ptr) {
        return count;
      }
      ++takeOut_ptr;
    }
    ++string;
    ++count;
  }
  return count;
}
