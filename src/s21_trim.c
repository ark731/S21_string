#include <stdlib.h>

#include "s21_string.h"

void *s21_trim(const char *src, const char *trim_chars) {
  char *result_string = NULL;
  if (src != NULL) {
    if ((trim_chars == NULL) || (s21_strlen(trim_chars) == 0)) {
      const char default_chars[7] = " \n\t\v\f\r";
      result_string = delete_symbols_from_src(src, default_chars);
    } else
      result_string = delete_symbols_from_src(src, trim_chars);
  }
  return (void *)result_string;
}

char *delete_symbols_from_src(const char *src, const char *trim_chars) {
  s21_size_t lenght_of_src = s21_strlen(src);
  s21_size_t left_index = 0;
  while (*src != '\0' && s21_strchr(trim_chars, *src) != NULL) {
    left_index++;
    src++;
  }
  lenght_of_src -= left_index;
  char *dest = (char *)malloc((lenght_of_src + 1) * sizeof(char));
  if (dest) {
    s21_strncpy(dest, src, lenght_of_src);
    dest[lenght_of_src] = '\0';
    if (lenght_of_src != 0) {
      while ((lenght_of_src != 0) &&
             s21_strchr(trim_chars, dest[lenght_of_src - 1]) != S21_NULL) {
        dest[lenght_of_src - 1] = '\0';
        lenght_of_src--;
      }
    }
  }
  return dest;
}