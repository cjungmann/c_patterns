/**
 * @file ltoa.c
 * @brief Implementation of a ltoa() function with comparisons to other methods.
 */

#include <alloca.h>
#include <assert.h>
#include <string.h>  // for strncpy
#include <limits.h>  // for LONG_MAX

/**
 * @defgroup MainContent
 * @brief Function group that performs the long-to-string conversion
 * @details
 *    Together, @ref ltoa and its supporting @ref ltoa_recursive_copy
 *    perform conversions from a long integer to a string value.  The
 *    number base can be specified from base-2 to base-36, with base
 *    specifiers outside of that range reverting to base-10.
 *
 *    No library functions are required to run these conversion
 *    functions.
 * @{
 */

/**
 * @brief Supporting @ref ltoa with recursive digit conversion
 * @details
 *    Recursion before printing digits will print the digits of the
 *    output string in the appropriate order.  By tracking the end
 *    character, we safely remain within the bounds of the allocated
 *    memory.
 * @param value    value from which the least significant digit will
 *                 be written as a number character.
 * @param base     number base in which the output will be written
 * @param ptr      pointer where the next number character should be
 *                 written
 * @param end      pointer to last permitted address for writing the
 *                 number variables.  The '\0' terminator will be
 *                 applied by the function that started the recursion.
 */
void ltoa_recursive_copy(unsigned long value, int base, char **ptr, char *end)
{
   if (value > 0)
   {
      // Recursion to reverse digits
      int placeval = value % base;
      ltoa_recursive_copy(value/base, base, ptr, end);

      if (*ptr < end)
      {
         int digit_addend = (placeval < 10) ? '0' : ('A' - 10);
         **ptr = placeval + digit_addend;
         ++(*ptr);
      }
   }
}

/**
 * @brief Uses recursion to safely converts a long integer into a string.
 * @details
 *    This is the featured function.  If you copy it for use,
 *    don't forget to copy @ref ltoa_recursive_copy with it.
 *
 *    If @b buffer is NULL or @b bufflen less than 2, the
 *    function will not do any transation, but the return
 *    value will still be the buffer length needed to
 *    create the long int translation.
 *
 * @param value    long value to convert to string
 * @param base     number base for conversion.  Allowed
 *                 values from 2 to 36, using base 10
 *                 for an out-of-range @b base value.
 * @param buffer   buffer to which the string will be written
 * @param bufflen  number of chars in @b buffer.
 *
 * @return Length of buffer needed to print the number.
 */
int ltoa_recursive(long value,
                   int base,
                   char *buffer,
                   int bufflen)
{
   if (base <= 0 || base > 36)
      base = 10;

   unsigned long uvalue;
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
      for (unsigned long lval = uvalue; lval > 0; lval /= base)
         ++required_length;
   }

   // Bufflen must be at least 1 to contain the NULL terminator
   if (buffer && bufflen > 1)
   {
      char *ptr = buffer;
      // '-1' to save room for termating '\0':
      char *end = buffer + bufflen - 1;

      if (negative)
         *ptr++ = '-';

      ltoa_recursive_copy(uvalue, base, &ptr, end);
      *ptr = '\0';
   }

   return required_length;
}


/**
 * @brief
 *    Implementation of ltoa function using a loop instead of recursion
 *
 * @details
 *    Like snprintf, this function always returns the length of the
 *    conversion, whether or not the conversion is completed.  One
 *    valid strategy is to call the function twice, first with a NULL
 *    @b buffer value to get the length needed, then call a second
 *    time with a buffer that had been allocated with the information
 *    provided by the first call.
 *
 * @param value    long value to be converted to a string
 * @param base     number base for conversion.  Allowed
 *                 values from 2 to 36, using base 10
 *                 for an out-of-range @b base value.
 * @param buffer   buffer to which output should be written
 * @param bufflen  length of @b buffer in bytes
 *
 * @return the number of characters needed to fully express
 *         the long @b value in the number @base specified
 */
int ltoa_loop(long value, int base, char *buffer, int bufflen)
{
   if (base <= 0 || base > 36)
      base = 10;

   unsigned long uvalue;
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
      for (unsigned long lval = uvalue; lval > 0; lval /= base)
         ++required_length;
   }

   // Bufflen must be at least 1 to contain the NULL terminator
   if (buffer && bufflen > 1)
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

         work_digit = uvalue % base;
         if (work_digit < 10)
            work_digit += '0';
         else
            work_digit += ('A' - 10);

         *ptr_digit = work_digit;

         --ptr_digit;
         uvalue /= base;
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

   return required_length;
}

/**
 * @brief
 *    Implementation of ltoa function reusing a static buffer for each call
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
 * @param value    long value to be converted to a string
 * @param base     number base for conversion.  Allowed
 *                 values from 2 to 36, using base 10
 *                 for an out-of-range @b base value.
 *
 * @return the number of characters needed to fully express
 *         the long @b value in the number @base specified
 */
const char *ltoa_instant(long value, int base)
{
   // Maximum length calculated for binary expression with a
   // termination '\0' AND possibly a sign character for a
   // negative value.
   //
   // Zero entire buffer to ensure final, string-terminating char,
   // is '\0', then we never have to touch it.
   static char buffer[2 + sizeof(long)*8] = { 0 };
   static char *buffend = buffer + sizeof(buffer) - 2;

   // This will always point to the next unconverted position.
   // Add 1 back to the address when returning to point to the
   // start of the converted string.
   char *cur_digit = buffend - 1;

   if (base <= 0 || base > 36)
      base = 10;

   if (value == 0)
      *cur_digit-- = '0';
   else
   {
      int negative = value < 0;

      unsigned long uvalue;
      if (negative)
         uvalue = -value;
      else
         uvalue = value;

      while (uvalue > 0)
      {
         assert(cur_digit >= buffer);
         char cval = uvalue % base;
         if (cval < 10)
            cval += '0';
         else
            cval += ('A' - 10);

         *cur_digit-- = cval;
         uvalue /= base;
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


/**
 * @defgroup TestingGroups
 * @brief Group of function groups with varying testing strategies
 * @details
 *    There are groups of functions that do performance testing
 *    on my `ltoa` function variations compared with the C library
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
typedef void (*LPRINTER)(long value);

/**
 * @brief Wrapper around long to string conversion using snprintf
 * @param value   value to convert
 */
void convert_with_snprintf(long value)
{
   int len = snprintf(NULL, 0, "%ld", value);
   char *buff = (char*)alloca(len);

   snprintf(buff, len, "%ld", value);
}

/**
 * @brief Wrapper around long to string conversion function that uses recursion
 * @param value   value to convert
 */
void convert_with_ltoa_recursive(long value)
{
   int len = ltoa_recursive(value, 10, NULL, 0);
   char *buff = (char*)alloca(len);

   ltoa_recursive(value, 10, buff, len);
}

/**
 * @brief Wrapper around long to string conversion function that uses a loop
 * @param value   value to convert
 */
void convert_with_ltoa_loop(long value)
{
   int len = ltoa_loop(value, 10, NULL, 0);
   char *buff = (char*)alloca(len);

   ltoa_loop(value, 10, buff, len);
}

/**
 * @brief Wrapper around long to string conversion function that uses a loop
 * @param value   value to convert
 */
void convert_with_ltoa_instant(long value)
{
   ltoa_instant(value, 10);
}

/**
 * @brief Call the function pointer to execute the test
 * @details
 *    For each of the previously generated long values in the
 *    @b lvals array, this function records how long it takes to
 *    run the function of the function pointer, printing a
 *    statistics report of the series of timings.
 *
 * @param lvals       array of long values
 * @param vals_count  number of long values in the array
 * @param prntr       pointer to wrapper function to test
 */
void run_timed_test(const long *lvals, int vals_count, LPRINTER prntr)
{
   // mark the array ending
   const long *end = lvals + vals_count;

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

/**
 * @brief
 *    Calls each of the conversion timer functions with the same
 *    set of long values.
 * @param  lvals    array of long values to test with each timer function
 * @param  len      number of long values in the @b lvals array
 */
void compare_conversion_strategies(long *lvals, int len)
{
   printf("\nTime %d conversions of various long values using "
          "\033[32;1m%s\e[39;22m\n",
          len, "ltoa_recursive");
   run_timed_test(lvals, len, convert_with_ltoa_recursive);

   printf("\nTime %d conversions of various long values using "
          "\033[32;1m%s\e[39;22m\n",
          len, "ltoa_loop");
   run_timed_test(lvals, len, convert_with_ltoa_loop);

   printf("\nTime %d conversions of various long values using "
          "\033[32;1m%s\e[39;22m\n",
          len, "ltoa_instant");
   run_timed_test(lvals, len, convert_with_ltoa_instant);

   printf("\nTime %d conversions of various long values using "
          "\033[32;1m%s\e[39;22m\n",
          len, "snprintf");
   run_timed_test(lvals, len, convert_with_snprintf);
}

/** @} end of group Method_NewMemory */


/**
 * @defgroup BaseTesting
 * @define Easily perform a battery of tests to confirm proper base handling.
 * @{
 */
void print_with_ltoa_base(long value, int base)
{
   int len = ltoa_loop(value, base, NULL, 0);
   char *buffer = (char*)alloca(len);
   if (buffer)
   {
      ltoa_loop(value, base, buffer, len);
      printf("   base %2d, %2d chars for '%s'\n",
             base, len, buffer);
   }
}

void test_the_value(long val)
{
   printf("Testing conversions for value %ld:\n", val);
   print_with_ltoa_base(val, 10);
   print_with_ltoa_base(val, 2);
   print_with_ltoa_base(val, 8);
   print_with_ltoa_base(val, 16);
   print_with_ltoa_base(val, 36);
}

void test_with_base(void)
{
   test_the_value(1000);
   test_the_value(0);
   test_the_value(LONG_MAX);
   test_the_value(LONG_MIN);
}

/** @} end of BaseTesting */

/**
 * @brief Populate an array of longs with random values
 * @details
 *    Makes a reusable list of long values so all of the
 *    performance tests can use the same long values.
 */
void initialize_array_of_longs(long *longs, int len)
{
   srandom(time(NULL));
   for (int i=0; i<len; ++i, longs++)
      *longs = random();
}


int main(int argc, const char **argv)
{
   printf("Size of long is %ld bytes, multiplied by 8 for bits in\n"
          "a binary expression is %ld.  Then, add 1 for '\\0' and\n"
          "one for possible negative for longest buffer requirement\n"
          "of %ld.\n\n",
          sizeof(long), sizeof(long)*8, 2+sizeof(long)*8);

   test_with_base();
   getchar();

   int sample_count = 10000;
   if (argc > 1)
   {
      char *endptr;
      long sc_arg = strtol(argv[1], &endptr, 10);
      if (endptr > argv[1])
         sample_count = sc_arg;
   }

   long *lvalues = (long*)alloca(sample_count * sizeof(long));
   if (lvalues)
   {
      initialize_array_of_longs(lvalues, sample_count);

      compare_conversion_strategies(lvalues, sample_count);

      // // test_with_base();
      // printf("Running test with per-conversion buffer allocation.\n");
      // compare_snprintf_ltoa(lvalues, sample_count);

      // printf("\n\nRunning test with single, external buffer allocation.\n");
      // compare_snprintf_ltoa_buffer(lvalues, sample_count);
   }
}

/* Local Variables:                 */
/* compile-command: "gcc           \*/
/*   -std=c99 -Wall -Werror -ggdb  \*/
/*   -fsanitize=address            \*/
/*   -lm                           \*/
/*   -o ltoa ltoa.c"                */
/* End:                             */
