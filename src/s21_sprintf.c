#include "s21_sprintf.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "s21_string.h"

/////////////////////////////////////////////////////////////////
/* ------------------ Core sprintf function ------------------ */
/////////////////////////////////////////////////////////////////
/* Core sprintf function. */
int s21_sprintf(char *str, const char *format, ...) {
  int returnCode = 0;
  char *cursor = str;  // writing into output string by this pointer
  va_list valist;
  va_start(valist, format);

  /* Go through format line symbol by symbol. If it's not '%', write to output
   * string. If it's '%%', write '%' to output string. If just one '%', call
   * parsing function, it will write specified argument into output string. */
  while (*format && !returnCode) {
    (*format != '%')
        ? (*cursor++ = *format++)
        : ((*++format == '%')
               ? (*cursor++ = *format++)
               : (returnCode = parseMain(&cursor, &format, str, &valist)));
  }
  returnCode ? cursor = str : 0;  // this will nullify output string on error
  *cursor = '\0';                 // finalize output string
  va_end(valist);
  /* If no error, return number of symbols written into output string. */
  return returnCode ? returnCode : (int)(cursor - str);
}

/////////////////////////////////////////////////////////////////
/* --- Functions to parse specifier format and set options --- */
/////////////////////////////////////////////////////////////////
/* Main function of this section. It will call option parsers and if specifier
 * format was correct, it will call writing function. */
static int parseMain(char **cursor, const char **format, const char *str,
                     va_list *valist) {
  options opt = {0};
  opt.str = str;  // remember startpoint for %n
  int returnCode;
  parseFlags(format, &opt);
  parseWidth(format, &opt, valist);
  parsePrecision(format, &opt, valist);
  parseLength(format, &opt);
  /* This is there we'll know if specifier format was valid: */
  returnCode = parseType(format, &opt.type);

  /* If it was valid, proceed with writing variable into output string. */
  !returnCode ? writeMain(cursor, &opt, valist) : 0;

  return returnCode;
}

/* Read flags. */
static void parseFlags(const char **format, options *opt) {
  while (1) {  // there can be many flags
    switch (**format) {
      case '-':
        (*opt).bitFlags |= S21_SPRINTF_FLAG_ALIGN_LEFT;
        break;
      case '+':
        (*opt).bitFlags |= S21_SPRINTF_FLAG_FORCE_SIGN;
        break;
      case ' ':
        (*opt).bitFlags |= S21_SPRINTF_FLAG_LEAVE_SPACE;
        break;
      case '#':
        (*opt).bitFlags |= S21_SPRINTF_FLAG_PRISON;
        break;
      case '0':
        (*opt).bitFlags |= S21_SPRINTF_FLAG_LEAD_ZEROES;
        break;
      default:
        return;  // this is the ONLY exit point
    }
    ++*format;
  }
}

/* Get width. */
static void parseWidth(const char **format, options *opt, va_list *valist) {
  switch (**format) {
    default:
      return;  // if no width given, go out
    case '*':  // negative width is read as positive width plus flag '-'
      int tmp = va_arg(*valist, int);
      if (tmp < 0) {
        (*opt).bitFlags |= S21_SPRINTF_FLAG_ALIGN_LEFT;
        (*opt).width = -tmp;
      } else {
        (*opt).width = tmp;
      }
      ++*format;
      break;
    case '0' ... '9':
      (*opt).width = readNumber(format);
      break;
  }
}

/* Get precision. */
static void parsePrecision(const char **format, options *opt, va_list *valist) {
  if (**format != '.') {
    return;  // if no precision option given, go out
  }
  /* We need flag to know if precision was actually set because its value
   * was initialised as 0. */
  (*opt).bitFlags |= S21_SPRINTF_PRECISION_SET;
  switch (*++*format) {
    case '0' ... '9':
      (*opt).precision = readNumber(format);
      break;
    case '*':
      int tmp = va_arg(*valist, int);
      if (tmp >= 0) {
        (*opt).precision = tmp;
      } else {  // negative precision value is ignored
        (*opt).bitFlags &= ~S21_SPRINTF_PRECISION_SET;
      }
      ++*format;
      break;
  }
  /* Limit precision to prevent stack overflow. */
  (*opt).precision > S21_SPRINTF_MAX_PRECISION
      ? (*opt).precision = S21_SPRINTF_MAX_PRECISION
      : 0;
}

/* Get type length. */
static void parseLength(const char **format, options *opt) {
  while (1) {  // technically we should support 'll' length hence the cycle
    switch (**format) {  // but it wasn't in the task so whatever
      case 'h':
        (*opt).bitFlags |= S21_SPRINTF_LENGTH_SHORT;
        break;
      case 'l':
        (*opt).bitFlags |= S21_SPRINTF_LENGTH_LONG;
        break;
      case 'L':
        (*opt).bitFlags |= S21_SPRINTF_LENGTH_LDBL;
        break;
      default:
        return;
    }
    ++*format;
  }
}

/* Finally get the specifier itself. */
static int parseType(const char **format, char *type) {
  const char *tmp = s21_strchr("cdeEfFgGinopsuxX", **format);
  int returnCode = ((tmp == S21_NULL) || *tmp == '\0') ? -1 : 0;
  !returnCode ? *type = *(*format)++
              : 0;  // not incrementing pointer past terminator
  return returnCode;
}

/////////////////////////////////////////////////////////////////
/* ------- Functions to write variables to output line ------- */
/////////////////////////////////////////////////////////////////
/* Main function of this section. It will call write functions corresponding to
 * specifier. */
static void writeMain(char **cursor, options *opt, va_list *valist) {
  switch ((*opt).type) {
    case 'c':
      writeChar(cursor, opt, valist);
      break;
    case 's':
      writeString(cursor, opt, valist);
      break;
    case 'e' ... 'g':
    case 'E' ... 'G':
      setupFloat(cursor, opt, valist);
      break;
    case 'd':
    case 'i':
      setupSigned(cursor, opt, valist);
      break;
    case 'o':
    case 'p':
    case 'u':
    case 'x':
    case 'X':
      setupUnsigned(cursor, opt, valist);
      break;
    case 'n':
      writeWritten(*cursor, opt, valist);
      break;
  }
}

/* Write char. */
static void writeChar(char **cursor, options *opt, va_list *valist) {
  /* If align left flag is on, print char first. */
  (IS_ALIGN_LEFT) ? *(*cursor)++ = (char)va_arg(*valist, int) : 0;
  writeFiller(cursor, &(*opt).width, 1, ' ');
  /* If align left flag is off, print char last. */
  !(IS_ALIGN_LEFT) ? *(*cursor)++ = (char)va_arg(*valist, int) : 0;
}

/* Write string. */
static void writeString(char **cursor, options *opt, va_list *valist) {
  const char *str = va_arg(*valist, char *);
  if (str == S21_NULL) {  // handle NULL pointer
    /* It should print (null) but only if precision allows full line. */
    str = (PRECISION_SET && ((*opt).precision < 6)) ? "" : "(null)";
  }

  s21_size_t len = s21_strlen(str);
  /* Calculate how many symbols will be written. */
  /* And how many of them will come from input string according to precision. */
  (PRECISION_SET && ((*opt).precision < len)) ? len = (*opt).precision : 0;

  /* If align left flag is on, concatenate first. */
  (IS_ALIGN_LEFT) ? memcpySpecial(cursor, str, 0, len) : 0;
  writeFiller(cursor, &(*opt).width, len, ' ');
  /* If align left flag is off, concatenate after. */
  !(IS_ALIGN_LEFT) ? memcpySpecial(cursor, str, 0, len) : 0;
}

/* Handling %n specifier. */
static void writeWritten(const char *cursor, options *opt, va_list *valist) {
  long int tmp = (long int)(cursor - (*opt).str);
  /* It actually understands length modifier so here it is. ¯\_(ツ)_/¯ */
  if ((*opt).bitFlags & S21_SPRINTF_LENGTH_LONG) {
    long int *written = va_arg(*valist, long int *);
    *written = tmp;
  } else if ((*opt).bitFlags & S21_SPRINTF_LENGTH_SHORT) {
    short *written = (short *)va_arg(*valist, int *);
    *written = (short)tmp;
  } else {
    int *written = va_arg(*valist, int *);
    *written = (int)tmp;
  }
}

/* Write out NaN. */
static void writeNAN(char **cursor, long double var, options *opt) {
  char *tmpBuffer = s21_strchr("EFG", (*opt).type) ? "NAN" : "nan";
  /* Check if it's negative NaN (yes, it is possible). */
  (__signbitl(var)) ? (*opt).bitFlags |= S21_SPRINTF_NEGATIVE_NUMBER : 0;
  (*opt).bitFlags &= ~S21_SPRINTF_FLAG_LEAD_ZEROES;
  writeFinal(cursor, opt, tmpBuffer);
}

/* Write out INF. */
static void writeINF(char **cursor, long double var, options *opt) {
  char *tmpBuffer = s21_strchr("EFG", (*opt).type) ? "FNI" : "fni";
  /* Add '-' sign for negative infinity. */
  (__signbitl(var)) ? (*opt).bitFlags |= S21_SPRINTF_NEGATIVE_NUMBER : 0;
  (*opt).bitFlags &= ~S21_SPRINTF_FLAG_LEAD_ZEROES;
  writeFinal(cursor, opt, tmpBuffer);
}

/* Write final line to output string. */
static void writeFinal(char **cursor, options *opt, char *str) {
  s21_size_t len = s21_strlen(str);
  if (IS_ALIGN_LEFT) { /* If align left flag is on, this block writes. */
    addPrefixOrSign(cursor, opt);  // write prefix for octal/hexadecimal
    s21_size_t tmp = (*opt).precision;
    if (INT_PRECISION_SET && (*opt).precision) {  // print '0's for
      writeFiller(cursor, &tmp, len, '0');        // precision in integer types
    }
    memcpySpecial(cursor, str, 1, len);  // write number from buffer, reversed

    s21_strchr("fFgGeE", (*opt).type) ? trimWidth(opt) : 0;
    if (!tmp) {  // I'm really sorry for these lines
      (*opt).precision ? --(*opt).precision : 0;
      writeFiller(cursor, &(*opt).width, (*opt).precision + len, ' ');
    } else {  // but it works and I'm too tired
      writeFiller(cursor, &(*opt).width,
                  (*opt).precision > len ? (*opt).precision : len, ' ');
    }
  } else {           /* If align left flag is off, this block writes. */
    if (HASFLAG_0) { /* If flag '0' is on, write prefix aligned left. */
      addPrefixOrSign(cursor, opt);
      s21_strchr("fFgGeE", (*opt).type) ? trimWidth(opt) : 0;
      writeFiller(cursor, &(*opt).width, len, '0');
    } else {
      if (INT_PRECISION_SET && (*opt).precision) {
        writeFiller(cursor, &(*opt).width, (*opt).precision, ' ');
      } else {  // really really sorry
        s21_strchr("fFgGeE", (*opt).type) ? trimWidth(opt) : 0;
        writeFiller(cursor, &(*opt).width, len, ' ');
      }
      addPrefixOrSign(cursor, opt);  // if no leading 0s, prexix goes offset
      if (INT_PRECISION_SET && (*opt).precision) {
        writeFiller(cursor, &(*opt).precision, len, '0');
      }
    }
    memcpySpecial(cursor, str, 1, len);  // write number from buffer, reversed
  }
}

/////////////////////////////////////////////////////////////////
/* ---- Auxiliary functions to prepare writing to output ----- */
/////////////////////////////////////////////////////////////////
/* Setup writing unsigned integer type variable. */
static void setupUnsigned(char **cursor, options *opt, va_list *valist) {
  if (PRECISION_SET) {
    (*opt).bitFlags &= ~S21_SPRINTF_FLAG_LEAD_ZEROES;
    (*opt).bitFlags |= S21_SPRINTF_INT_PRECISION_SET;
  }
  long unsigned int var;
  long unsigned int base = getBase(opt);
  /* We need buffer long enough to hold decimal representation of integer
   * with most digits possible. For us it'll be octal UINT64_MAX. */
  char tmpBuffer[S21_BUFFER_FOR_LONGEST_OCTAL_UINT] = {0};
  /* Pull variable of correct size from arguments. */
  if ((*opt).type == 'p') {
    var = (long unsigned int)va_arg(*valist, void *);
    var ? (*opt).bitFlags |= S21_SPRINTF_FLAG_PRISON : 0;
  } else if ((*opt).bitFlags & S21_SPRINTF_LENGTH_LONG) {
    var = va_arg(*valist, long unsigned int);
  } else if ((*opt).bitFlags & S21_SPRINTF_LENGTH_SHORT) {
    var = (short unsigned int)va_arg(*valist, unsigned int);
  } else {
    var = va_arg(*valist, unsigned int);
  }
  /* Turn off prefix printing if var == 0. */
  !var && (*opt).type != 'p' ? (*opt).bitFlags &= ~S21_SPRINTF_FLAG_PRISON : 0;
  /* Include 0x/0 prefixes in print width calc. */
  trimWidthByBase(opt, base);
  if ((*opt).type == 'p' && !var) {
    writeFinal(cursor, opt, ")lin(");
  } else {
    /* Convert number to string in reverse order. */
    char *nums = ((*opt).type == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
    if (var || (PRECISION_SET && !(*opt).precision)) {
      for (int i = 0; var; ++i) {
        *(tmpBuffer + i) = *(nums + var % base);
        var /= base;
      }
    } else {
      *tmpBuffer = '0';
    }
    writeFinal(cursor, opt, tmpBuffer);  // finally call write function
  }
}

/* Setup writing signed decimal type variable. */
static void setupSigned(char **cursor, options *opt, va_list *valist) {
  if (PRECISION_SET) {
    (*opt).bitFlags &= ~S21_SPRINTF_FLAG_LEAD_ZEROES;
    (*opt).bitFlags |= S21_SPRINTF_INT_PRECISION_SET;
  }
  long int var;
  (*opt).type = 'd';  // %i does the same so we combine them here
  /* If it's good enough for octal UINT64, it's good here as well. */
  char tmpBuffer[S21_BUFFER_FOR_LONGEST_OCTAL_UINT] = {0};

  /* Pull variable of correct size from arguments. */
  if ((*opt).bitFlags & S21_SPRINTF_LENGTH_LONG) {
    var = va_arg(*valist, long int);
  } else if ((*opt).bitFlags & S21_SPRINTF_LENGTH_SHORT) {
    var = (short int)va_arg(*valist, int);
  } else {
    var = va_arg(*valist, int);
  }
  /* We have to add '-' sign manually. */
  (var < 0) ? (*opt).bitFlags |= S21_SPRINTF_NEGATIVE_NUMBER : 0;
  trimWidth(opt);
  /* Convert number to string in reverse order. */
  if (var || (PRECISION_SET && !(*opt).precision)) {
    for (int i = 0; var; ++i) {
      *(tmpBuffer + i) = labs(var % 10) + 48;
      var /= 10;
    }
  } else {
    *tmpBuffer = '0';
  }

  writeFinal(cursor, opt, tmpBuffer);  // call write function
}

/* Setup writing floating point type variable. */
static void setupFloat(char **cursor, options *opt, va_list *valist) {
  long double var = ((*opt).bitFlags & S21_SPRINTF_LENGTH_LDBL)
                        ? va_arg(*valist, long double)
                        : va_arg(*valist, double);
  if (isnan(var)) { /* If NaN, handle it. */
    writeNAN(cursor, var, opt);
  } else if (isinf(var)) { /* If not NaN, check for infinities. */
    writeINF(cursor, var, opt);
  } else { /* If not NaN/INF, go here and handle actual numbers. */
    /* Set default precision if it wasn't set. */
    !PRECISION_SET ? (*opt).precision = 6 : 0;
    if (__signbitl(var)) { /* Check if the number is negative. */
      var = -var;
      (*opt).bitFlags |= S21_SPRINTF_NEGATIVE_NUMBER;
    } /* Now call converting function according to specifier. */
    ((*opt).type == 'g' || (*opt).type == 'G') ? convertSmart(cursor, opt, var)
    : ((*opt).type == 'f' || (*opt).type == 'F')
        ? convertFloat(cursor, opt, var)
        : convertExponent(cursor, opt, var);
  }
}

/* Convert float type variable to string. */
static void convertFloat(char **cursor, options *opt, long double var) {
  /* Buffer must hold max precision + longest possible integer part of LDBL.
   */
  char tmpBuffer[S21_SPRINTF_MAX_PRECISION + LDBL_MAX_10_EXP + 50];
  char *tmpCursor = tmpBuffer;
  if (!(*opt).precision) { /* Handle simple case of precision 0. */
    var = rintl(var);
    HAS_PRISON ? *tmpCursor++ = '.' : 0;
    longDoubleToString(&tmpCursor, var, 1, opt);
  } else { /* Otherwise split number into parts. */

    int leadingZeroes;       // need these to offset significant digits
    int trailingZeroes = 0;  // of fractional part correctly
    long double integ;
    long double fract = splitLongDoubleFloat(var, &integ, &leadingZeroes, opt);
    trailingZeroes = writeTrailingZeroes(&tmpCursor, leadingZeroes, opt);
    if (IS_TYPE_G && !fract) {  // %#g prints delimiting '.' even if fractional
      HAS_PRISON ? *tmpCursor++ = '.' : 0;  // part is 0
    } else {
      (*opt).bitFlags |= S21_SPRINTF_PRINTING_FRACT;
      longDoubleToString(&tmpCursor, fract, (*opt).precision - trailingZeroes,
                         opt);
    }
    longDoubleToString(&tmpCursor, integ, 1, opt);
  }
  writeFinal(cursor, opt, tmpBuffer);
}

/* Convert float type variable to string in scientific format. */
static void convertExponent(char **cursor, options *opt, long double var) {
  (*opt).bitFlags |= S21_SPRINTF_PRINTING_EXPONENT;
  /* Buffer must hold max precision + some. */
  char tmpBuffer[S21_SPRINTF_MAX_PRECISION + 50];
  char *tmpCursor = tmpBuffer;
  /* Normalize and find decimal exponent for %e formatting. */
  if (var) {  // watch for var == 0
    var = normalize(var, &(*opt).exponent, 1);
  }
  if (!(*opt).precision) { /* Handle simple case of precision 0. */
    var = rintl(var);
  }

  int leadingZeroes;       // need these to offset significant digits
  int trailingZeroes = 0;  // of fractional part correctly
  long double integ;
  long double fract = splitLongDouble(var, &integ, &leadingZeroes, opt);

  if (integ >= 10.0L) {  // check for overflow
    integ /= 10.0L;
    ++(*opt).exponent;
  }
  writeExponent(&tmpCursor, opt);

  trailingZeroes = writeTrailingZeroes(&tmpCursor, leadingZeroes, opt);
  /* Call function to convert both parts to one string in reverse. */
  if (!(*opt).precision || (!fract && IS_TYPE_G && ((*opt).exponent >= 0))) {
    HAS_PRISON ? *tmpCursor++ = '.' : 0;  // %#g prints delimiting '.' even
  } else {                                // if fractional part is 0
    (*opt).bitFlags |= S21_SPRINTF_PRINTING_FRACT;
    longDoubleToString(&tmpCursor, fract, (*opt).precision - trailingZeroes,
                       opt);
  }
  longDoubleToString(&tmpCursor, integ, 1, opt);

  writeFinal(cursor, opt, tmpBuffer);
}

/* Choose convertation format for %g specifier. */
static void convertSmart(char **cursor, options *opt, long double var) {
  long double tmpvar = var;
  /* Set flag indicating we're working with %g. */
  ((*opt).bitFlags |= S21_SPRINTF_SMART);

  /* Find P for formula  P > X ≥ −4 */
  int p = (*opt).precision ? (*opt).precision : 1;
  /* Get exponent. */
  if (tmpvar) {  // watch for var == 0
    tmpvar = normalize(tmpvar, &(*opt).exponent, 1);
  }
  if (!(*opt).precision) {  // round if necessary
    tmpvar = rintl(tmpvar);
  }
  /* if P > X ≥ −4, the conversion is with style %f and precision P − (X + 1).
   * Otherwise, the conversion is with style %e and precision P − 1. */
  (*opt).precision ? --(*opt).precision : 0;
  int leadingZeroes;
  long double integ;
  (void)splitLongDouble(tmpvar, &integ, &leadingZeroes, opt);
  if (integ >= 10.0L) {
    ++(*opt).exponent;
  }
  /* Decide which format to use. */
  if ((p > (*opt).exponent) && ((*opt).exponent >= -4)) {
    (*opt).precision = ((int)(*opt).precision > (*opt).exponent)
                           ? (*opt).precision - (*opt).exponent + 1
                           : 0;
    (*opt).precision ? --(*opt).precision : 0;
    convertFloat(cursor, opt, var);
  } else {
    convertExponent(cursor, opt, var);
  }
}

/* Write base 8/16 prefix or signed type sign or leading space. */
static int addPrefixOrSign(char **cursor, options *opt) {
  char *symbolsAdded = *cursor;
  /* Base 8/16 prefixes. */
  if (HAS_PRISON) {
    switch ((*opt).type) {
      case 'o':
        *(*cursor)++ = '0';
        break;
      case 'p':
      case 'x':
        *(*cursor)++ = '0';
        *(*cursor)++ = 'x';
        break;
      case 'X':
        *(*cursor)++ = '0';
        *(*cursor)++ = 'X';
        break;
    }
  }
  /* Sign or leading space. */
  if (s21_strchr("defgEFG", (*opt).type)) {  // only for these specifiers
    IS_NEGATIVE    ? *(*cursor)++ = '-'
    : HASFLAG_PLUS ? *(*cursor)++ = '+'
    : HASFLAG_SPC  ? *(*cursor)++ = ' '
                   : 0;
  }
  return (int)(*cursor - symbolsAdded);
}

/* Print trailing zeroes past significant digits. */
static int writeTrailingZeroes(char **tmpCursor, int leadingZeroes,
                               options *opt) {
  /* Calculate number of zeroes needed for normalized output. */
  int trailingZeroes = 0;
  if ((*opt).precision > S21_SPRINTF_FP_CALC_MAX_DIGITS) {
    trailingZeroes =
        (int)(*opt).precision - S21_SPRINTF_FP_CALC_MAX_DIGITS + leadingZeroes;
    if (!(IS_TYPE_G && !(HAS_PRISON))) {
      for (int i = 0; i < trailingZeroes; ++i) {
        *(*tmpCursor)++ = '0';
      }
    }
  }
  return trailingZeroes;
}

/* Write filler spaces or zeroes to output string by external counter. */
static void writeFiller(char **cursor, s21_size_t *from, s21_size_t to,
                        char filler) {
  while ((*from)-- > to) {
    *(*cursor)++ = filler;
  }
}

/* Write exponent for scientfic notation. */
static void writeExponent(char **tmpCursor, options *opt) {
  int exp = (*opt).exponent;
  char sign = '+';
  if (exp < 0) {
    exp = -exp;
    sign = '-';
  }
  int i = 0;
  while (exp) {
    *(*tmpCursor)++ = (exp % 10) + 48;
    exp /= 10;
    ++i;
  }
  while (i < 2) {  // exponent always has at least 2 digits
    *(*tmpCursor)++ = '0';
    ++i;
  }
  *(*tmpCursor)++ = sign;
  *(*tmpCursor)++ = (s21_strchr("EG", (*opt).type)) ? 'E' : 'e';
}

/////////////////////////////////////////////////////////////////
/* ------------------- Auxiliary functions ------------------- */
/////////////////////////////////////////////////////////////////
/* Set base to 8, 10 or 16 according to specifier. */
static long unsigned int getBase(options *opt) {
  return ((*opt).type == 'u') ? 10 : ((*opt).type == 'o') ? 8 : 16;
}

/* Read unsigned decimal number from string. */
static s21_size_t readNumber(const char **format) {
  s21_size_t num = 0;
  while (**format >= '0' && **format <= '9') {
    num *= 10;
    num += *(*format)++ - 48;
  }
  return num;
}

/* Decimal normalization to specified position. */
static long double normalize(long double var, int *exponent, int pos) {
  *exponent = (int)roundl(log10l(var)) - pos;
  // printf("<%13d %3d %Le>", *exponent, pos, var); //////////////////////////
  var /= powl(10.0L, (long double)*exponent);
  /* log10 is not exactly decimal exponent, adjust it. */
  if (var < powl(10.0L, pos - 1)) {
    var *= 10.0L;
    --*exponent;
  }
  if (var >= powl(10.0L, pos)) {
    var /= 10.0L;
    ++*exponent;
  }
  return var;
}

/* If nan/inf were cast to int, nullify it. */
static int trimExtremeValues(int exponent) {
  return ((exponent < -10000000) || (exponent > 10000000)) ? 0 : exponent;
}

/* Include sign or replacing space in print width calc. */
static void trimWidth(options *opt) {
  (IS_NEGATIVE || HASFLAG_PLUS || HASFLAG_SPC)
      ? (*opt).width ? --(*opt).width : 0
      : 0;
}

/* Include 0x/0 prefixes in print width calc with this beautiful check. */
static void trimWidthByBase(options *opt, long unsigned int base) {
  ((*opt).width >= 2) ? (HAS_PRISON) ? (base == 16)  ? (*opt).width -= 2
                                       : (base == 8) ? (*opt).width -= 1
                                                     : 0
                                     : 0
                      : 0;
}

/* Copy up to len number of bytes from str to cursor, possible in reverse
 * order. Set cursor to next byte after copied array. */
static void memcpySpecial(char **cursor, const char *str, int reverse,
                          s21_size_t len) {
  if (reverse) {
    for (s21_size_t i = len; i; --i) {
      *(*cursor)++ = *(str + i - 1);  // reverse
    }
  } else {
    for (s21_size_t i = 0; (i < len); ++i) {
      *(*cursor)++ = *str++;  // direct
    }
  }
}

/* Split number into integral and fractional parts, shift fractional left,
 * round number with bankers' rounding. Function for %e-style output. */
static long double splitLongDouble(long double var, long double *integ,
                                   int *rightExp, options *opt) {
  long double fract = modfl(var, integ);  // split number
  /* Store decimal exponent of fractional part and set it to 0. */
  fract = normalize(fract, rightExp, 0);
  *rightExp = trimExtremeValues(*rightExp);
  /* Shift fractional left to precision or limit for easy rounding. */
  long double shiftRight =
      powl(10.0L, fminl((long double)(*opt).precision,
                        (long double)S21_SPRINTF_FP_CALC_MAX_DIGITS));
  fract *= shiftRight;        // finalize shifting
  fract = rintl(fract);       // bankers' rounding
  if (fract >= shiftRight) {  // if fractional part spills into integral,
    *integ += 1;              // handle it here
    fract -= shiftRight;
  }
  return fract;
}

/* Split number into integral and fractional parts, shift fractional left,
 * round number with bankers' rounding. Function for %f-style output. */
static long double splitLongDoubleFloat(long double var, long double *integ,
                                        int *rightExp, options *opt) {
  long double fract = modfl(var, integ);  // split number
  /* Store decimal exponent of fractional part and set it to 0. */
  fract = normalize(fract, rightExp, 0);
  *rightExp = trimExtremeValues(*rightExp);
  /* Another offset if precision trims significant digits. */
  if ((*opt).precision < S21_SPRINTF_FP_CALC_MAX_DIGITS) {
    fract = normalize(fract, rightExp, *rightExp);
  }
  /* Store exponent in settings to calculate required number of filler '0's
   * later. */
  if (*rightExp < 0) {
    (*opt).exponent = *rightExp;
  }
  /* Shift fractional left to precision or limit for easy rounding. */
  long double shiftRight =
      powl(10.0L, fminl((long double)(*opt).precision,
                        (long double)S21_SPRINTF_FP_CALC_MAX_DIGITS));
  fract *= shiftRight;   // finalize shifting
  fract = rintl(fract);  // bankers' rounding
  if ((fract >= shiftRight) && (*rightExp > -1)) {
    *integ += 1;          // if fractional part spills into integral,
    fract -= shiftRight;  // handle it here
  }
  return fract;
}

/* Write integral part of long double variable to string. Set pointer to '\0'
 */
static void longDoubleToString(char **buffer, long double var, s21_size_t len,
                               options *opt) {
  s21_size_t counter = 0;
  if (IS_FRACT && (len > (*opt).precision)) {  // trim fractional to precision
    for (s21_size_t i = (*opt).precision; i < len; ++i) {
      var /= 10.0L;
      ++counter;
    }
    var = rintl(var);
  } /* This is one very specific edge case that will disable convertation. */
  if (!(IS_TYPE_G && IS_FRACT && (!var || isnan(var)) && !HAS_PRISON)) {
    /* This will remove trailing '0's from fractional part for %g specifier.
     */
    if (IS_TYPE_G && IS_FRACT && !HAS_PRISON) {
      int exit = 1;
      while (exit && (var >= 1.L)) {
        if ((int)fmodl(var, 10.L)) {  // until we meet significant number
          exit = 0;
        } else {
          var /= 10.0L;  // simply divide by 10
          ++counter;
        }
      }
    }
    while (var >= 1.L) {  // get digit and put it in string
      *(*buffer)++ = (int)fmodl(var, 10.L) + 48;
      var /= 10.0L;
      ++counter;
    }
    while (counter++ < len) {  // fill the rest with '0's
      *(*buffer)++ = '0';
    }
    if (IS_FRACT) {  // and put '.' if we're writing fractional part
      *(*buffer)++ = '.';
    }
  }
  (*opt).bitFlags &= ~S21_SPRINTF_PRINTING_FRACT;  // only printing fract once
  **buffer = '\0';                                 // finalize
}