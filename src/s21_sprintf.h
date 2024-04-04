#include <stdarg.h>

#include "s21_string.h"

/* Main options object. */
typedef struct options {
  int bitFlags;          // Flags storage
  char type;             // specifier type
  const char *str;       // pointer to start of output string
  s21_size_t width;      // minimum field width
  s21_size_t precision;  // precision of conversion
  int exponent;          // decimal exponent of fp number
} options;

/* Aliases for some flag checks. */
#define IS_ALIGN_LEFT ((*opt).bitFlags & S21_SPRINTF_FLAG_ALIGN_LEFT)
#define HAS_PRISON ((*opt).bitFlags & S21_SPRINTF_FLAG_PRISON)
#define HASFLAG_PLUS ((*opt).bitFlags & S21_SPRINTF_FLAG_FORCE_SIGN)
#define HASFLAG_SPC ((*opt).bitFlags & S21_SPRINTF_FLAG_LEAVE_SPACE)
#define HASFLAG_0 ((*opt).bitFlags & S21_SPRINTF_FLAG_LEAD_ZEROES)
#define IS_NEGATIVE ((*opt).bitFlags & S21_SPRINTF_NEGATIVE_NUMBER)
#define IS_FRACT ((*opt).bitFlags & S21_SPRINTF_PRINTING_FRACT)
#define IS_TYPE_G ((*opt).bitFlags & S21_SPRINTF_SMART)
#define PRECISION_SET ((*opt).bitFlags & S21_SPRINTF_PRECISION_SET)
#define INT_PRECISION_SET ((*opt).bitFlags & S21_SPRINTF_INT_PRECISION_SET)
#define PRINT_EXP ((*opt).bitFlags & S21_SPRINTF_PRINTING_EXPONENT)

/* Flags to set type width. */
#define S21_SPRINTF_LENGTH_SHORT 01  // short integer type
#define S21_SPRINTF_LENGTH_LONG 02   // long integer or double type
#define S21_SPRINTF_LENGTH_LDBL 04   // long double type

/* Specifier flags. */
#define S21_SPRINTF_FLAG_ALIGN_LEFT 0100    // flag '-'
#define S21_SPRINTF_FLAG_FORCE_SIGN 0200    // flag '+'
#define S21_SPRINTF_FLAG_LEAVE_SPACE 0400   // flag ' '
#define S21_SPRINTF_FLAG_PRISON 01000       // flag '#'
#define S21_SPRINTF_FLAG_LEAD_ZEROES 02000  // flag '0'

/* Auxiliary flags. */
/* Precision was set explicitly. */
#define S21_SPRINTF_PRECISION_SET 010
/* Precision set with integer type */
#define S21_SPRINTF_INT_PRECISION_SET 020
/* Number we convert was negative. */
#define S21_SPRINTF_NEGATIVE_NUMBER 040
/* Function was called to convert fractional part of floating point number. */
#define S21_SPRINTF_PRINTING_FRACT 04000
/* We're converting with %g/%G specifier. */
#define S21_SPRINTF_SMART 010000
/* We're converting number to exponential notation. */
#define S21_SPRINTF_PRINTING_EXPONENT 020000

/* Size of string to hold octal representation of longest unsigned integer
 * we support. 23 is enough for UINT64_MAX plus ending '\0'. */
#define S21_BUFFER_FOR_LONGEST_OCTAL_UINT 23

/* Set limit for precision value taken from specifier. Only limited by stack
 * size. 4950 is enough to represent longest precise value in %f format. */
#define S21_SPRINTF_MAX_PRECISION 100000

/* Set precision for floating point calculations. 17 is guaranteed to have no
 * trailing garbage. 18 is mostly precise and allows minimal garbage. Larger
 * values provide rounding errors more often than correct values. */
#define S21_SPRINTF_FP_CALC_MAX_DIGITS 17

#ifndef LDBL_MAX_10_EXP
/* Not including <float.h> for just one value. */
#define LDBL_MAX_10_EXP 4932
#endif

/////////////////////////////////////////////////////////////////
/* --- Functions to parse specifier format and set options --- */
/////////////////////////////////////////////////////////////////
/* Main function of this section. It will call option parsers and if specifier
 * format was correct, it will call writing function. */
static int parseMain(char **cursor, const char **format, const char *str,
                     va_list *valist);

/* Read flags. */
static void parseFlags(const char **format, options *opt);

/* Get width. */
static void parseWidth(const char **format, options *opt, va_list *valist);

/* Get precision. */
static void parsePrecision(const char **format, options *opt, va_list *valist);

/* Get type length. */
static void parseLength(const char **format, options *opt);

/* Get type. */
static int parseType(const char **format, char *type);

/////////////////////////////////////////////////////////////////
/* ------- Functions to write variables to output line ------- */
/////////////////////////////////////////////////////////////////
/* Main function of this section. It will call write functions corresponding to
 * specifier. */
static void writeMain(char **cursor, options *opt, va_list *valist);

/* Write char. */
static void writeChar(char **cursor, options *opt, va_list *valist);

/* Write string. */
static void writeString(char **cursor, options *opt, va_list *valist);

/* Handle %n specifier. */
static void writeWritten(const char *cursor, options *opt, va_list *valist);

/* Write out NaN. */
static void writeNAN(char **cursor, long double var, options *opt);

/* Write out INF. */
static void writeINF(char **cursor, long double var, options *opt);

/* Write final line to output string. */
static void writeFinal(char **cursor, options *opt, char *str);

/////////////////////////////////////////////////////////////////
/* ---- Auxiliary functions to prepare writing to output ----- */
/////////////////////////////////////////////////////////////////
/* Setup writing unsigned integer type variable. */
static void setupUnsigned(char **cursor, options *opt, va_list *valist);

/* Setup writing signed decimal type variable. */
static void setupSigned(char **cursor, options *opt, va_list *valist);

/* Setup writing floating point type variable. */
static void setupFloat(char **cursor, options *opt, va_list *valist);

/* Convert float type variable to string. */
static void convertFloat(char **cursor, options *opt, long double var);

/* Convert float type variable to string in scientific format. */
static void convertExponent(char **cursor, options *opt, long double var);

/* Choose convertation format for %g specifier. */
static void convertSmart(char **cursor, options *opt, long double var);

/* Write base 8/16 prefix or signed type sign or leading space. */
static int addPrefixOrSign(char **cursor, options *opt);

/* Print trailing zeroes past significant digits. */
static int writeTrailingZeroes(char **tmpCursor, int leadingZeroes,
                               options *opt);

/* Write filler spaces or zeroes to string by external counter. */
static void writeFiller(char **cursor, s21_size_t *from, s21_size_t to,
                        char filler);

/* Write exponent for scientific notation. */
static void writeExponent(char **tmpCursor, options *opt);

/////////////////////////////////////////////////////////////////
/* ------------------- Auxiliary functions ------------------- */
/////////////////////////////////////////////////////////////////
/* Set base to 8, 10 or 16 according to specifier. */
static long unsigned int getBase(options *opt);

/* Read unsigned decimal number from string. */
static s21_size_t readNumber(const char **format);

/* Decimal normalization to specified position. */
static long double normalize(long double var, int *exponent, int pos);

/* If nan/inf were cast to int, nullify it. */
static int trimExtremeValues(int exponent);

/* Include sign or replacing space in print width calc. */
static void trimWidth(options *opt);

/* Include 0x/0 prefixes in print width calc. */
static void trimWidthByBase(options *opt, long unsigned int base);

/* Copy up to len number of bytes from str to cursor, possible in reverse
 * order. Set cursor to next byte after copied array. */
static void memcpySpecial(char **cursor, const char *str, int reverse,
                          s21_size_t len);

/* Split number into integral and fractional parts, shift fractional left, round
 * number with bankers' rounding. Function for %e-style output. */
static long double splitLongDouble(long double var, long double *integ,
                                   int *rightExp, options *opt);
/* Split number into integral and fractional parts, shift fractional left, round
 * number with bankers' rounding. Function for %f-style output. */
static long double splitLongDoubleFloat(long double var, long double *integ,
                                        int *rightExp, options *opt);

/* Write integral part of long double variable to string. Set pointer to '\0'
 */
static void longDoubleToString(char **buffer, long double var, s21_size_t len,
                               options *opt);