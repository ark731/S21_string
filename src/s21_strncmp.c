#include "s21_string.h"

int s21_strncmp(const char *string1, const char *string2, s21_size_t number) {
  s21_size_t i = 0;
  int result = 0;
  while (i < number && (string1[i] != '\0' || string2[i] != '\0')) {
    if (string1[i] != string2[i]) {
      result = string1[i] - string2[i];
      i = number;
    }
    i++;
  }

  return result;
}