#include "s21_string.h"

char *s21_strpbrk(const char *str1, const char *str2) {
  int flag = 0;
  char *result = S21_NULL;

  for (int j = 0; str1[j] != '\0' && flag == 0; j++) {
    for (int i = 0; str2[i] != '\0'; i++) {
      if (str1[j] == str2[i]) {
        flag = 1;
        result = (char *)str1 + j;
      }
    }
  }
  return result;
}