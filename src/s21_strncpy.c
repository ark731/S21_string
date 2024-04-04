#include "s21_string.h"

char *s21_strncpy(char *dest, const char *src, s21_size_t n) {
  s21_size_t count = 0;
  while ((count < n) && (*src != '\0')) {
    dest[count] = *src;
    src++;
    count++;
  }
  dest[count] = 0;
  return dest;
}