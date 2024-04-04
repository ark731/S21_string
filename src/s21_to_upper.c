#include <stdlib.h>

#include "s21_string.h"

void* s21_to_upper(const char* str) {
  if (!str) return S21_NULL;
  s21_size_t length = s21_strlen(str);
  char* new_string = (char*)malloc((length + 1) * sizeof(char));

  for (s21_size_t i = 0; i < length; i++) {
    if (str[i] >= 'a' && str[i] <= 'z') {
      new_string[i] = str[i] - ('a' - 'A');
    } else {
      new_string[i] = str[i];
    }
  }
  new_string[length] = '\0';

  return new_string;
}