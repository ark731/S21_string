#include "s21_string.h"

char *s21_strstr(const char *text, const char *pattern) {
  char *start = S21_NULL;
  int flag = 1;
  while (flag && *text != '\0') {
    const char *current_text = text;
    const char *entry = pattern;

    while (*entry != '\0' && *current_text == *entry) {
      entry++;
      current_text++;
    }

    if (*entry == '\0') {
      start = (char *)text;
      flag = 0;
    }

    text++;
  }

  return start;
}
