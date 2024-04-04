#include "s21_string.h"

int s21_memcmp(const void* str1, const void* str2, s21_size_t n) {
  if (!n) return 0;
  unsigned const char* str1_copy = (unsigned const char*)str1;
  unsigned const char* str2_copy = (unsigned const char*)str2;

  int difference = 0;
  if (*str1_copy == *str2_copy) {
    s21_size_t count = 1;
    do {
      str1_copy++;
      str2_copy++;
      count++;
      difference = *str1_copy - *str2_copy;
    } while ((*str1_copy == *str2_copy) && (count < n) && (difference == 0));
  } else
    difference = *str1_copy - *str2_copy;

  return difference;
}