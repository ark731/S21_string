#include "s21_string.h"

void* s21_memchr(const void* str, int c, s21_size_t n) {
  unsigned const char* returned_value = (unsigned const char*)str;
  unsigned char s = (unsigned char)c;
  s21_size_t count = 0;

  while ((*returned_value != s) && (count < n)) {
    returned_value++;
    count++;
  }
  if (count == n) {
    returned_value = S21_NULL;
  }
  return (void*)returned_value;
}