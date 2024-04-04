#include <stdlib.h>

#include "s21_string.h"

int isUpperLetter(char c) {
  if (((int)c >= (int)'A') && ((int)c <= (int)'Z'))
    return 1;
  else
    return 0;
}

void* s21_to_lower(const char* str) {
  if (!str) return S21_NULL;
  s21_size_t length = s21_strlen(str);
  char* new_string = (char*)malloc((length + 1) * sizeof(char));

  for (s21_size_t i = 0; i < length; i++) {
    if (isUpperLetter(str[i])) {
      new_string[i] = str[i] - ('A' - 'a');
    } else {
      new_string[i] = str[i];
    }
  }
  new_string[length] = '\0';

  return new_string;
}