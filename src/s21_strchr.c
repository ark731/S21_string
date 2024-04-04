#include "s21_string.h"

char *s21_strchr(const char *string, int symbol) {
  const char *current_symbol = string;
  const char *res = S21_NULL;
  int flag = 0;
  char s = (char)symbol;
  while (*current_symbol != '\0' && !flag) {
    if (*current_symbol == s) {
      res = current_symbol;
      flag++;
    }
    current_symbol++;
  }
  if (s == '\0') res = current_symbol;
  return (char *)res;
}