#include "s21_string.h"

char *s21_strtok(char *str, const char *delim) {
  static char *pointer;
  static s21_size_t len;
  char *returned_value;
  char *current_symbol;
  if (str == S21_NULL) {
    current_symbol = pointer;
    returned_value = S21_NULL;
  } else {
    current_symbol = str;
    pointer = str;
    len = s21_strlen(str);
  }

  if (!len) {
    returned_value = S21_NULL;
  } else {
    while (*current_symbol != '\0' &&
           s21_strchr(delim, *current_symbol) != S21_NULL) {
      current_symbol++;
      pointer++;
      len--;
    }
    returned_value = pointer;
    while (*current_symbol != '\0' &&
           s21_strchr(delim, *current_symbol) == S21_NULL) {
      current_symbol++;
      pointer++;
      len--;
    }
    *current_symbol = '\0';
    pointer++;
    len ? len-- : 0;
  }
  return returned_value;
}