/**
 * @file ltoa.c
 * @brief Implementation of a ltoa() function with comparisons to other methods.
 */

#include <alloca.h>
#include <assert.h>
#include <string.h>  // for strncpy
#include <limits.h>  // for INT_MAX, SHRT_MAX, LONG_MAX, etc

// typedef short ITYPE;
// typedef unsigned short UITYPE;
// #define TYPESPEC "%d"
// #define ITYPE_MAX SHRT_MAX
// #define ITYPE_MIN SHRT_MIN

// typedef int ITYPE;
// typedef unsigned int UITYPE;
// #define TYPESPEC "%d"
// #define ITYPE_MAX INT_MAX
// #define ITYPE_MIN INT_MIN

typedef long ITYPE;
typedef unsigned long UITYPE;
#define TYPESPEC "%ld"
#define ITYPE_MAX LONG_MAX
#define ITYPE_MIN LONG_MIN

// typedef long long ITYPE;
// typedef unsigned long long UITYPE;
// #define TYPESPEC "%lld"
// #define ITYPE_MAX LLONG_MAX
// #define ITYPE_MIN LLONG_MIN

/**
 * @defgroup MainContent
 * @brief Function group that performs the integer-to-string conversion
 * @details
 *    Together, @ref itoa_recursive and its supporting
 *    @ref itoa_recursive_copy perform conversions from an integer to
 *    a string value.  The @b radix argument can be specified from base-2
 *    to base-36, with radix specifiers outside of that range reverting
 *    to base-10.
 *
 *    No library functions are required to run these conversion
 *    functions.
 * @{
 */

/**
 * @brief Supporting @ref itoa_recursive with recursive digit conversion
 * @details
 *    Recursion before printing digits will print the digits of the
 *    output string in the appropriate order.  By tracking the end
 *    character, we safely remain within the bounds of the allocated
 *    memory.
 * @param value    value from which the least significant digit will
 *                 be written as a number character.
 * @param radix    number base in which the output will be written
 * @param ptr      pointer where the next number character should be
 *                 written
 * @param end      pointer to last permitted address for writing the
 *                 number variables.  The '\0' terminator will be
 *                 applied by the function that started the recursion.
 */
void itoa_recursive_copy(UITYPE value, int radix, char **ptr, char *end)
{
   if (value > 0)
   {
      // Recursion to reverse digits
      int placeval = value % radix;
      itoa_recursive_copy(value/radix, radix, ptr, end);

      if (*ptr < end)
      {
         int digit_addend = (placeval < 10) ? '0' : ('A' - 10);
         **ptr = placeval + digit_addend;
         ++(*ptr);
      }
   }
}

/**
 * @brief Uses recursion to safely converts an integer into a string.
 * @details
 *    This is the featured function.  If you copy it for use,
 *    don't forget to copy @ref itoa_recursive_copy with it.
 *
 *    If @b buffer is NULL or @b bufflen less than 2, the
 *    function will not do any transation, but the return
 *    value will still be the buffer length needed to
 *    create the integer translation.
 *
 * @param value    integer value to convert to string
 * @param radix    number base for conversion.  Allowed
 *                 values from 2 to 36, using base 10
 *                 for an out-of-range @b radix value.
 * @param buffer   buffer to which the string will be written
 * @param bufflen  number of chars in @b buffer.
 *
 * @return Length of buffer needed to print the number.
 */
int itoa_recursive(ITYPE value,
                   int radix,
                   char *buffer,
                   int bufflen)
{
   if (radix <= 0 || radix > 36)
      radix = 10;

   UITYPE uvalue;
   int negative = 0;
   int required_length;
   if (value == 0)
   {
      required_length = 2;
      uvalue = 0;
   }
   else
   {
      if (value < 0)
      {
         negative = 1;
         uvalue = -value;
      }
      else
         uvalue = value;

      // Required length is the return value regardless
      // of buffer size (like snprintf)
      required_length = negative ? 2 : 1;
      for (UITYPE lval = uvalue; lval > 0; lval /= radix)
         ++required_length;
   }

   // Bufflen must be at least 1 to contain the NULL terminator
   if (buffer && bufflen > 1)
   {
      if (value==0)
         memcpy(buffer, "0", 2);
      else
      {
         char *ptr = buffer;
         // '-1' to save room for termating '\0':
         char *end = buffer + bufflen - 1;

         if (negative)
            *ptr++ = '-';

         itoa_recursive_copy(uvalue, radix, &ptr, end);
         *ptr = '\0';
      }
   }

   return required_length;
}


/**
 * @brief
 *    Implementation of itoa function using a loop instead of recursion
 *
 * @details
 *    Like snprintf, this function always returns the length of the
 *    conversion, whether or not the conversion is completed.  One
 *    valid strategy is to call the function twice, first with a NULL
 *    @b buffer value to get the length needed, then call a second
 *    time with a buffer that had been allocated with the information
 *    provided by the first call.
 *
 * @param value    integer value to be converted to a string
 * @param radix    number base for conversion.  Allowed
 *                 values from 2 to 36, using base 10
 *                 for an out-of-range @b radix value.
 * @param buffer   buffer to which output should be written
 * @param bufflen  length of @b buffer in bytes
 *
 * @return the number of characters needed to fully express
 *         the ITYPE @b value in the number @base specified
 */
int itoa_loop(ITYPE value, int radix, char *buffer, int bufflen)
{
   if (radix <= 0 || radix > 36)
      radix = 10;

   UITYPE uvalue;
   int negative = 0;
   int required_length;
   if (value == 0)
   {
      required_length = 2;
      uvalue = 0;
   }
   else
   {
      if (value < 0)
      {
         negative = 1;
         uvalue = -value;
      }
      else
         uvalue = value;

      // Required length is the return value regardless
      // of buffer size (like snprintf)
      required_length = negative ? 2 : 1;
      for (UITYPE lval = uvalue; lval > 0; lval /= radix)
         ++required_length;
   }

   // Bufflen must be at least 1 to contain the NULL terminator
   if (buffer && bufflen > 1)
   {
      if (value==0)
         memcpy(buffer, "0", 2);
      else
      {
         char work_digit;

         // Prepare working memory and pointers into it
         char *work_buffer = (char*)alloca(required_length);
         char *ptr_digit = work_buffer + required_length-1;
         *ptr_digit-- = '\0';

         while (uvalue > 0)
         {
            // Since we've calculated exactly how many characters will be
            // needed, this condition is redundant, assertion for safety:
            assert(ptr_digit >= work_buffer);

            work_digit = uvalue % radix;
            if (work_digit < 10)
               work_digit += '0';
            else
               work_digit += ('A' - 10);

            *ptr_digit = work_digit;

            --ptr_digit;
            uvalue /= radix;
         }

         if (negative)
         {
            // Since we've calculated exactly how many characters will be
            // needed, this condition is redundant, assertion for safety:
            assert(ptr_digit >= work_buffer);

            *ptr_digit-- = '-';
         }

         strncpy(buffer, work_buffer, bufflen);
      }
   }

   return required_length;
}

/**
 * @brief
 *    Implementation of itoa function reusing a static buffer for each call
 *
 * @details
 *    This function returns the string result, a pointer into the
 *    static buffer.  Since digits of an integer value are deciphered
 *    from least to most significant digits, the numbers are written
 *    to the buffer starting at the end and working backwards.  When
 *    finished translation, the pointer to the last-written digit is
 *    returned as the _answer_.
 *
 *    Note that since this function is working with and returning
 *    a static buffer, there is no need for the calling function to
 *    provide a buffer.
 *
 * @param value    integer value to be converted to a string
 * @param radix    number base for conversion.  Allowed
 *                 values from 2 to 36, using base 10
 *                 for an out-of-range @b radix value.
 *
 * @return the number of characters needed to fully express
 *         the ITYPE @b value in the number @b radix specified
 */
const char *itoa_instant(ITYPE value, int radix)
{
   // Maximum length calculated for binary expression with a
   // termination '\0' AND possibly a sign character for a
   // negative value.
   //
   // Zero entire buffer to ensure final, string-terminating char,
   // is '\0', then we never have to touch it.
   static char buffer[2 + sizeof(ITYPE)*8] = { 0 };
   static char *buffend = buffer + sizeof(buffer) - 2;

   // This will always point to the next unconverted position.
   // Add 1 back to the address when returning to point to the
   // start of the converted string.
   char *cur_digit = buffend - 1;

   if (radix <= 0 || radix > 36)
      radix = 10;

   if (value == 0)
      *cur_digit-- = '0';
   else
   {
      int negative = value < 0;

      UITYPE uvalue;
      if (negative)
         uvalue = -value;
      else
         uvalue = value;

      while (uvalue > 0)
      {
         assert(cur_digit >= buffer);
         char cval = uvalue % radix;
         if (cval < 10)
            cval += '0';
         else
            cval += ('A' - 10);

         *cur_digit-- = cval;
         uvalue /= radix;
      }

      if (negative)
         *cur_digit-- = '-';
   }

   return cur_digit + 1;
}

/** @} end of MainContent */

// The include statements here are only needed for the testing
// functions that follow.

#define _OPEN_SOURCE 500  // enable snprintf in stdio.h
#include <stdio.h>
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED
#endif
#include <stdlib.h>    // random
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>      // for time() to seed random number, timespec

// c_patterns/perftest.c for timing:
#define PT_INCLUDE_IMPLEMENTATIONS
#define PT_INCLUDE_RESULTS_REPORT
#include "perftest.c"

#include <limits.h>
#include <stdbool.h>
#include <alloca.h>
#include <locale.h>


/**
 * @defgroup TestingGroups
 * @brief Group of function groups with varying testing strategies
 * @details
 *    There are groups of functions that do performance testing
 *    on my `itoa` function variations compared with the C library
 *    function `snprintf`, which is the builtin way to do it.
 */

/**
 * @ingroup TestingGroups
 * @defgroup Method_NewMemory
 * @brief Adhoc memory allocation method
 * @details
 *    Each conversion measures and allocates memory for its conversion.
 *    This strategy allocates exactly the required amount of memory
 *    for each conversion to perform memory-safe conversions.
 *
 *    The conversion functions neither use nor save the conversion
 *    results.
 * @{
 */

/** Typedef with which @ref run_timed_test runs convert functions */
typedef void (*LPRINTER)(ITYPE value);

/**
 * @brief Wrapper around integer-to-string conversion using snprintf
 * @param value   value to convert
 */
void convert_with_snprintf(ITYPE value)
{
   int len = snprintf(NULL, 0, TYPESPEC, value);
   char *buff = (char*)alloca(len);

   snprintf(buff, len, TYPESPEC, value);
}

/**
 * @brief Wrapper around integer-to-string conversion function that uses recursion
 * @param value   value to convert
 */
void convert_with_itoa_recursive(ITYPE value)
{
   int len = itoa_recursive(value, 10, NULL, 0);
   char *buff = (char*)alloca(len);

   itoa_recursive(value, 10, buff, len);
}

/**
 * @brief Wrapper around integer-to-string conversion function that uses a loop
 * @param value   value to convert
 */
void convert_with_itoa_loop(ITYPE value)
{
   int len = itoa_loop(value, 10, NULL, 0);
   char *buff = (char*)alloca(len);

   itoa_loop(value, 10, buff, len);
}

/**
 * @brief Wrapper around integer-to-string conversion function to static buffer
 * @param value   value to convert
 */
void convert_with_itoa_instant(ITYPE value)
{
   itoa_instant(value, 10);
}

/**
 * @brief Wrapper to use itoa_instant to copy to new string buffer.
 * @details
 *    itoa_instant overwrites a static buffer with each call, so
 *    the result is transient.  itoa_instant skips the steps of
 *    predicting memory requirements and memory allocation needed
 *    by the snprintf, itoa_recursion, and itoa_loop methods.
 *
 *    This wrapper measures, allocates, and copies the result of
 *    itoa_instant to fairly compare performance with the other
 *    methods for non-transient results.
 * @param value   value to convert
 */
void convert_with_itoa_instant_copy(ITYPE value)
{
   const char *result = itoa_instant(value, 10);
   int len = strlen(result);
   char *buff = (char*)alloca(len);
   memcpy(buff, result, len);
}
/**
 * @brief Call the function pointer to execute the test
 * @details
 *    For each of the previously generated ITYPE values in the
 *    @b lvals array, this function records how long it takes to
 *    run the function of the function pointer, printing a
 *    statistics report of the series of timings.
 *
 * @param lvals       array of ITYPE values
 * @param vals_count  number of ITYPE values in the array
 * @param prntr       pointer to wrapper function to test
 */
void run_timed_test(const ITYPE *lvals, int vals_count, LPRINTER prntr)
{
   // mark the array ending
   const ITYPE *end = lvals + vals_count;

   int intervals_count = vals_count + 1;
   PT_Gettime_premem pte;
   PT_Gettime_premem_init(&pte, intervals_count);

   PerfTest *pt = (PerfTest*)&pte;

   PT_add_point(pt, NULL);
   while (lvals < end)
   {
      (*prntr)(*lvals);
      PT_add_point(pt, NULL);
      ++lvals;
   }

   pt_test_report(pt);

   PT_clean(pt);
}

int COL_TITLE = 36;
int COL_METHOD = 34;
int COL_VALUE = 0;

/**
 * @brief
 *    Calls each of the conversion timer functions with the same
 *    set of ITYPE values.
 * @param  lvals    array of ITYPE values to test with each timer function
 * @param  len      number of ITYPE values in the @b lvals array
 */
void compare_conversion_strategies(ITYPE *lvals, int len)
{
   char *old_locale = setlocale(LC_NUMERIC,NULL);
   setlocale(LC_NUMERIC, "");

   printf("\033[%d;1m"
          "Display by-method timings for a set of "
          "\033[%d;1m"
          "%'d"
          "\033[%d;1m"
          " values"
          "\e[39;22m\n",
          COL_TITLE, COL_VALUE, len, COL_TITLE);

   printf("\nConversion method \033[%d;1m%s\033[39;22m:\n",
          COL_METHOD,
          "snprintf");
   run_timed_test(lvals, len, convert_with_snprintf);

   printf("\nConversion method \033[%d;1m%s\033[39;22m:\n",
          COL_METHOD,
          "itoa_recursive");
   run_timed_test(lvals, len, convert_with_itoa_recursive);

   printf("\nConversion method \033[%d;1m%s\033[39;22m:\n",
          COL_METHOD,
          "itoa_loop");
   run_timed_test(lvals, len, convert_with_itoa_loop);

   printf("\nConversion method \033[%d;1m%s\033[39;22m:\n",
          COL_METHOD,
          "itoa_instant");
   run_timed_test(lvals, len, convert_with_itoa_instant);

   printf("\nConversion method \033[%d;1m%s\033[39;22m:\n",
          COL_METHOD,
          "itoa_instant_copy");
   run_timed_test(lvals, len, convert_with_itoa_instant_copy);

   setlocale(LC_NUMERIC, old_locale);
}

/** @} end of group Method_NewMemory */


/**
 * @defgroup BaseTesting
 * @define Easily perform a battery of tests to confirm proper radix handling.
 * @{
 */
/**
 * @brief
 *    For the value and radix, print results for various methods.
 * @details
 *    Print results for recursive method, loop method, and instant
 *    method for the given value, base arguments.
 * @param value    value to convert to string
 * @param radix    number base to use in conversion
 */
void print_with_itoa_radix(ITYPE value, int radix)
{
   int len = itoa_loop(value, radix, NULL, 0);
   char *recurse_buffer = (char*)alloca(len);
   char *loop_buffer = (char*)alloca(len);
   if (recurse_buffer && loop_buffer)
   {
      itoa_recursive(value, radix, recurse_buffer, len);
      itoa_loop(value, radix, loop_buffer, len);

      printf("   radix \033[34;1m%2d\033[39;22m required %2d chars, results by method\n"
             "      recursion: %s\n"
             "           loop: %s\n"
             "        instant: %s\n",
             radix, len,
             recurse_buffer,
             loop_buffer,
             itoa_instant(value, radix));
   }
}

/**
 * @brief For given value, run test for representative bases.
 * @param val   value to test
 */
void test_the_value(ITYPE val)
{
   printf("Testing conversions for value \033[33;1m"
          TYPESPEC
          "\033[39;22m:\n",
          val);
   print_with_itoa_radix(val, 10);
   print_with_itoa_radix(val, 2);
   print_with_itoa_radix(val, 8);
   print_with_itoa_radix(val, 16);
   print_with_itoa_radix(val, 36);
}

/**
 * @brief Run conversion tests of representative values
 * @details
 *    Test for typical value with 1000, then special cases
 *    0, ITYPE_MAX, and ITYPE_MIN to confirm proper handling
 *    at extreme values.
 */
void test_with_bases(void)
{
   test_the_value(1000);
   test_the_value(0);
   test_the_value(ITYPE_MAX);
   test_the_value(ITYPE_MIN);
}

/** @} end of BaseTesting */

/**
 * @brief Populate an array of integers with random values
 * @details
 *    Makes a reusable list of integer values so all of the
 *    performance tests can use the same ITYPE values.
 */
void initialize_array_of_integers(ITYPE *longs, int len)
{
   srandom(time(NULL));
   for (int i=0; i<len; ++i, longs++)
      *longs = random();
}

void display_max_buffer_explanation(void)
{
   printf("\033[1mExplain calculation of maximum required buffer size.\033[22m\n");
   printf("Size of our integer is %ld bytes, multiplied by 8 for bits in\n"
          "a binary expression is %ld.  Then, add 1 for '\\0' and\n"
          "one for possible negative for longest buffer requirement\n"
          "of %ld.\n\n",
          sizeof(ITYPE), sizeof(ITYPE)*8, 2+sizeof(ITYPE)*8);
}

void perform_timing_tests(int argc, const char **argv)
{
   int sample_count = 10000;
   if (argc > 1)
   {
      char *endptr;
      long sc_arg = strtol(argv[1], &endptr, 10);
      if (endptr > argv[1])
         sample_count = sc_arg;
   }

   ITYPE *lvalues = (ITYPE*)alloca(sample_count * sizeof(ITYPE));
   if (lvalues)
   {
      initialize_array_of_integers(lvalues, sample_count);

      compare_conversion_strategies(lvalues, sample_count);
   }
}


int main(int argc, const char **argv)
{
   printf("\033[H\033[2J");
   display_max_buffer_explanation();
   getchar();

   // printf("\033[H\033[2J");
   // test_with_bases();
   // getchar();

   printf("\033[H\033[2J");
   perform_timing_tests(argc, argv);
}

/* Local Variables:                 */
/* compile-command: "gcc           \*/
/*   -std=c99 -Wall -Werror -ggdb  \*/
/*   -fsanitize=address            \*/
/*   -lm                           \*/
/*   -o itoa itoa.c"                */
/* End:                             */
