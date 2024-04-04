
#include "s21_string.h"

char *s21_strncat(char *dest, const char *src, s21_size_t n) {
  s21_size_t index_of_src = 0;
  s21_size_t index_of_dest = s21_strlen(dest);
  while ((*src != '\0') && (index_of_src < n)) {
    dest[index_of_dest + index_of_src] = *src;
    index_of_src++;
    src++;
  }
  dest[index_of_dest + index_of_src] = '\0';
  return dest;
}