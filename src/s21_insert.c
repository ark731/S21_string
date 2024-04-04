#include <stdlib.h>

#include "s21_string.h"

void *s21_insert(const char *src, const char *str, s21_size_t start_index) {
  char *dest = S21_NULL;
  if ((src != S21_NULL) && (start_index <= s21_strlen(src))) {
    s21_size_t length_of_src = s21_strlen(src);
    s21_size_t length_of_str = (str) ? s21_strlen(str) : 0;
    dest = (char *)malloc((length_of_src + length_of_str + 1) * sizeof(char));
    if (dest) {
      dest[length_of_src + length_of_str] = '\0';
      s21_strncpy(dest, src, start_index);
      s21_size_t i;
      for (i = 0; i < length_of_str; i++) {
        dest[i + start_index] = str[i];
      }
      for (i = start_index; i < length_of_src; i++) {
        dest[length_of_str + i] = src[i];
      }
    }
  }
  return (void *)dest;
}