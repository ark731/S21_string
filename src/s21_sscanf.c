#include "s21_sscanf.h"

int s21_sscanf(const char *str, const char *format, ...) {
  flags f = {0};
  f.string_start = str;  // save the beginnig of the string for %n
  va_list p_arg;
  va_start(p_arg, format);
  parse_strings(str, format, &f, p_arg);
  printf("result is: %d\n", f.res);
  va_end(p_arg);
  return f.res;
}

void parse_strings(const char *str, const char *format, flags *f,
                   va_list p_arg) {
  for (; *str && *format && !f->err;) {
    if (is_space(*format)) {
      while (is_space(*format)) format++;
      while (is_space(*str)) str++;  // clear spaces in both strings
    } else if (*format == '%' && *(format + 1) == '%') {
      if (*str == '%') {
        format += 2;
        str++;
      }

    } else if (*format == '%') {
      format++;
      parse_specifier(&format, &f);
      write_in_var(&str, f, p_arg);
    } else if (*format == *str) {
      format++;
      str++;
    } else
      return;
  }
}

void write_char(const char **string, va_list arg, flags *f) {
  if (!f->ast) {
    *va_arg(arg, char *) = **string;
    f->res++;
  }
  (*string)++;
}

void write_int(const char **string, va_list arg, flags *f) {
  if (f->s == 'i' || f->s == 'u' || f->s == 'x' || f->s == 'X' || f->s == 'o' ||
      f->s == 'p') {
    while (**string && !is_space(**string)) (*string)++;
    if (!f->ast) {
      *va_arg(arg, int *) = 0;
      f->res++;
    }
  } else {
    int temp = 0;
    int sign = 0;
    if (**string == '-') {
      sign = 1;
      (*string)++;
    }
    if (**string == '+') (*string)++;
    while (is_digit(**string)) {
      temp = temp * 10 + (**string - 48);
      (*string)++;
    }
    if (!f->ast) {
      *va_arg(arg, int *) = sign ? -temp : temp;
      f->res++;
    }
  }
}
void write_fl(const char **string, va_list arg, flags *f) {
  while (**string && !is_space(**string)) (*string)++;
  if (!f->ast) {
    *va_arg(arg, double *) = 0.0;
    f->res++;
  }
}

void write_str(const char **string, va_list arg, flags *f) {
  char buff[2048] = {'\0'};
  int len = 0;
  while (**string && !is_space(**string)) {
    buff[len] = **string;
    len++;
    (*string)++;
  }
  if (!f->ast) {
    strcpy(va_arg(arg, char *), buff);  // TODO: change to s21
    f->res++;
  }
}

void write_in_var(const char **str, flags *f, va_list p_arg) {
  switch (f->s) {
    case 'c':
      write_char(str, p_arg, f);
      break;
    case 'd':
    case 'i':
    case 'u':
    case 'x':
    case 'X':
    case 'o':
    case 'p':
      clear_spaces_in_string(str);
      write_int(str, p_arg, f);
      break;
    case 'f':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
      clear_spaces_in_string(str);
      write_fl(str, p_arg, f);
      break;
    case 's':
      clear_spaces_in_string(str);
      write_str(str, p_arg, f);
      break;
    case 'n':
      *va_arg(p_arg, int *) = *str - f->string_start;
      break;
  }
}

int is_space(char c) { return (c == ' ' || c == '\t' || c == '\n'); }

int is_digit(char c) { return (c >= 48 && c <= 57); }

void parse_specifier(const char **c, flags **f) {
  (*f)->ast = 0;
  (*f)->w = 0;
  (*f)->s = 0;
  (*f)->l = 0;

  if (**c == '*') {
    (*f)->ast = 1;
    (*c)++;
  }
  while (!strchr(VALID_SPECIFIERS, **c)) {
    if (is_digit(**c))
      (*f)->w = ((*f)->w * 10) + **c - 48;
    else
      (*f)->l = **c;
    (*c)++;
  }
  (*f)->s = **c;
  (*c)++;
}

void clear_spaces_in_string(const char **string) {
  while (is_space(**string)) (*string)++;
}