/**
 * @file ltoa.c
 * @brief Implementation of a ltoa() function with comparisons to other methods.
 */

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
 */
void ltoa_recursive_copy(long value,   /**<
                                        * value from which the least significant
                                        * digit will be extracted, and during
                                        * recursion, the number from which the
                                        * least significant digit wasnn discarded
                                        * through division by @b base.
                                        */
                         int base,     /**< See #ref ltoa::base */
                         char **ptr,   /**<
                                        * Pointer to address of next character to
                                        * print.  The address is used and incremented
                                        * upon return from recursion, resulting in the
                                        * last-acquired, most significant digit being
                                        * printed at the front of the string.
                                        */
                         char *end    /**<
                                       * pointer to last permitted address for
                                       * printing digits.  The '\0' terminator will
                                       * be applied by the caller that initiated the
                                       * recursion.
                                       */
   )
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
 * @brief Safely converts a long integer into a string.
 * @details
 *    This is the featured function.  If you copy it for use,
 *    don't forget to copy @ref ltoa_recursive_copy with it.
 *
 *    If @b buffer is NULL or @b bufflen less than 2, the
 *    function will not do any transation, but the return
 *    value will still be the buffer length needed to
 *    create the long int translation.
 *
 * @return Length of buffer needed to print the number.
 */
int ltoa(long value,   /**< value to convert to a string */
         int base,     /**<
                        * number base to use.  Allowed base values are
                        * from 2 through 36.  Base values outside the
                        * permitted range will revert to base 10.
                        */
         char *buffer, /**<
                        * Pointer to buffer to which the string will
                        * be written.  It must be at least 1-character
                        * long, but ideally should be long enough to
                        * contain all the digits.
                        *
                        * No translation will happen if this value
                        * is NULL.
                        */
         int bufflen   /**<
                        * Number of chars in the @b buffer
                        *
                        * No translation will happen if this value
                        * is less than 2.
                        **/
   )
{
   if (base==0 || base > 36)
      base = 10;

   int negative = value < 0;
   if (negative)
      value = -value;

   // Required length is the return value regardless
   // of buffer size (like snprintf)
   int required_length = negative ? 2 : 1;
   for (long lval = value; lval > 0; lval /= base)
      ++required_length;

   // Bufflen must be at least 1 to contain the NULL terminator
   if (buffer && bufflen > 1)
   {
      char *ptr = buffer;
      // '-1' to save room for termating '\0':
      char *end = buffer + bufflen - 1;

      if (negative)
         *ptr++ = '-';

      ltoa_recursive_copy(value, base, &ptr, end);
      *ptr = '\0';
   }

   return required_length;
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
#define PT_INCLUDE_IMPLEMENATAIONS
#define PT_INCLUDE_GENERIC_TEST_REPORT
#include "perftest.c"

#include <limits.h>
#include <stdbool.h>
#include <alloca.h>

/**
 * @defgroup TestingGroups
 * @brief Group of function groups with varying testing strategies
 * @details
 *    There are groups of functions that do performance testing
 *    on my `ltoa` function compared with the C library function
 *    `snprintf`, which is the builtin way to do it.
 */

/**
 * @ingroup TestingGroups
 * @defgroup Method_NewMemory
 * @brief Adhoc memory allocation method
 * @details
 *    Each conversion measures and allocates memory for its conversion.
 *    This strategy allocates exactly the required amount of memory
 *    for each conversion to perform memory-safe conversions.
 * @{
 */

typedef void (*LPRINTER)(long value);

void print_with_snprintf(long value)
{
   int len = snprintf(NULL, 0, "%ld", value);
   char *buff = (char*)alloca(len);

   snprintf(buff, len, "%ld", value);
}

void print_with_ltoa(long value)
{
   int len = ltoa(value, 10, NULL, 0);
   char *buff = (char*)alloca(len);

   ltoa(value, 10, buff, len);
}

void run_timed_test(const long *lvals, int vals_count, LPRINTER prntr)
{
   // mark the array ending
   const long *end = lvals + vals_count;

   PT_Gettime_extmem pte;
   PerfTest *pt = (PerfTest*)&pte;
   PT_GTLink *links = (PT_GTLink*)alloca((vals_count+1) * PT_BLOCK_SIZE(ptr));
   PT_GTLink *lptr = links;

   PT_add_point(pt, *lptr++);
   while (lvals < end)
   {
      (*prntr)(*lvals);
      PT_add_point(pt, *lptr++);
      ++lvals;
   }


}

void compare_snprintf_ltoa(long *lvals, int len)
{
   printf("Begin timed stringify test of %d values using %s.\n",
          len, "ltoa");
   run_timed_test(lvals, len, print_with_ltoa);

   printf("Begin timed stringify test of %d values using %s.\n",
          len, "snprintf");
   run_timed_test(lvals, len, print_with_snprintf);
}

/** @} end of group Method_NewMemory */

/**
 * @ingroup TestingGroups
 * @defgroup Method_ReuseMemory
 * @brief A large-enough buffer is preemptively allocated to be reused for each conversion.
 * @details
 *    A block of memory is allocated that can hold the string
 *    representation of thelargest possible long value, and that
 *    buffer is passed to a version of conversion functions that
 *    will use the shared memory.  This removes the memory
 *    allocation step in the conversion, and thus we can compare
 *    the cost of memory allocation on the overall process time.
 * @{
 */

typedef void (*LPRINTER_B)(long value, char *buffer, int bufflen);

void print_with_snprintf_buff(long value, char *buffer, int bufflen)
{
   snprintf(buffer, bufflen, "%ld", value);
}

void print_with_ltoa_buff(long value, char *buffer, int bufflen)
{
   ltoa(value, 10, buffer, bufflen);
}

void run_timed_test_buffer(const long *lvals,
                           int vals_count,
                           char *buffer,
                           int bufflen,
                           LPRINTER_B prntr)
{
   // Mark the array ending
   const long *end = lvals + vals_count;

   struct timespec ts_start, ts_end;
   clock_gettime(CLOCK_MONOTONIC, &ts_start);
   while (lvals < end)
   {
      (*prntr)(*lvals, buffer, bufflen);
      ++lvals;
   }
   clock_gettime(CLOCK_MONOTONIC, &ts_end);

   printf("Run time was %ld time units.\n", ts_end.tv_nsec - ts_start.tv_nsec);
}

void compare_snprintf_ltoa_buffer(long *lvals, int len)
{
   // Prepare buffer than can handle the largest possible
   // long value and allow all calls to use it
   int max_len = ltoa(LONG_MAX, 10, NULL, 0);
   char *buffer = (char*)alloca(max_len);

   printf("Begin timed stringify test of %d values using %s.\n",
          len, "ltoa");
   run_timed_test_buffer(lvals, len, buffer, max_len, print_with_ltoa_buff);

   printf("Begin timed stringify test of %d values using %s.\n",
          len, "snprintf");
   run_timed_test_buffer(lvals, len, buffer, max_len, print_with_snprintf_buff);
}


/** @} end of group Method_ReuseMemory */

/**
 * @defgroup BaseTesting
 * @define Easily perform a battery of tests to confirm proper base handling.
 * @{
 */
void print_with_ltoa_base(long value, int base)
{
   int len = ltoa(value, base, NULL, 0);
   char *buffer = (char*)alloca(len);
   if (buffer)
   {
      ltoa(value, base, buffer, len);
      printf("For value %ld, base %d, we needed %d characters to output '%s'.\n",
             value, base, len, buffer);
   }
}

void test_with_base(void)
{
   print_with_ltoa_base(1000, 10);
   print_with_ltoa_base(1000, 2);
   print_with_ltoa_base(1000, 8);
   print_with_ltoa_base(1000, 16);
   print_with_ltoa_base(1000, 36);
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
   int sample_count = 1000;
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

      // test_with_base();
      printf("Running test with per-conversion buffer allocation.\n");
      compare_snprintf_ltoa(lvalues, sample_count);

      printf("\n\nRunning test with single, external buffer allocation.\n");
      compare_snprintf_ltoa_buffer(lvalues, sample_count);
   }
}

/* Local Variables:                 */
/* compile-command: "gcc           \*/
/*   -std=c99 -Wall -Werror -ggdb  \*/
/*   -fsanitize=address            \*/
/*   -o ltoa ltoa.c"                */
/* End:                             */
