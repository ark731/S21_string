#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define VALID_SPECIFIERS "cdieEfgGosuxXpn"

typedef struct {
  int ast;
  int w;
  char l;
  char s;
  int res;
  const char *string_start;
  int err;
} flags;

int s21_sscanf(const char *str, const char *format, ...);
void parse_strings(const char *str, const char *format, flags *f,
                   va_list p_arg);
void parse_specifier(const char **c, flags **f);
void write_in_var(const char **str, flags *f, va_list p_arg);
void write_int(const char **string, va_list arg, flags *f);
void write_char(const char **string, va_list arg, flags *f);
void write_fl(const char **string, va_list arg, flags *f);
void write_str(const char **string, va_list arg, flags *f);
void clear_spaces_in_string(const char **string);
int is_space(char c);
int is_digit(char c);