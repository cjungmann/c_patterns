/** @file */

#include <string.h>   // for memcpy
#include "sarray.h"

/**
 * @brief Used by sarray_measure_string to track strings.
 *
 * Tracks number of strings and cumulative length in order to
 * predict the memory block necessary to build an array of
 * string pointers to copies of the strings.
 */
typedef struct sarray_tally {
   int text_len;      ///< Total length of all substrings, including terminating '\0'.
   int string_count;  ///< Tally of substrings
} sarray_tally;


/**
 * @brief Holds information needed for the iterative initialization of the handle.
 */
typedef struct sarray_frame {
   char *buffer;          ///< memory to initialize and install into the handle
   sarray_handle *handle; ///< pointer to caller's handle or saving the result
} sarray_frame;

/**
 * @brief Called for each string during the measuring phase
 */
static void sarray_measure_string(const char *start, int length, void *data)
{
   sarray_tally *st = (sarray_tally*)data;
   ++st->string_count;
   st->text_len += length + 1;
}

static void sarray_handle_add_string(const char *start, int length, void *data)
{
   sarray_frame *sf = (sarray_frame*)data;
   sarray_handle *handle = sf->handle;

   // Copy string ahead of last string in buffer
   sf->buffer -= length+1;
   memcpy(sf->buffer, start, length);
   sf->buffer[length] = '\0';

   // install the copied string to the handle's string array
   handle->strings[handle->count] = sf->buffer;

   ++handle->count;
}


/**
 * @brief Calculate size of block of memory needed to construct the handle
 * @return required number of characters in buffer
 */
int sarray_measure(const char *string, walker_func func)
{
   sarray_tally tally = { 0, 0 };
   (*func)(string, sarray_measure_string, &tally);

   return (tally.string_count * sizeof(char*)) + tally.text_len;
}

/**
 * @brief Populate a sarray_handle object with data for array of strings
 * @param [in,out] handle          Handle to which string array is installed
 * @param [in]     string          String to transform
 * @param [in]     func            String-walker function to extract substrings
 * @param [in]     buffer          Pointer to block of memory into which the array
 *                                 and its component strings will be copied.
 * @param [in]     bufflen         what it says
 */
void sarray_build(sarray_handle *handle,
                  const char *string,
                  walker_func func,
                  char *buffer,
                  int bufflen)
{
   handle->count = 0;
   handle->strings = (const char**)buffer;

   sarray_frame sf = { buffer + bufflen, handle };

   (*func)(string, sarray_handle_add_string, &sf);
}

/**
 * @brief Convenient function safely returning a string from the string array.
 *
 * Returns a NULL if the index is out-of-range, so it's a quick
 * and safe way to access contents by incorporating a test along
 * with the access.
 * @param handle    handle of string array to search
 * @param index     0-based index into the array
 * @return Requested element if in range, NULL if not in range
 */
const char *sarray_element_by_index(sarray_handle *handle, unsigned index)
{
   if (index < handle->count)
      return handle->strings[index];
   else
      return NULL;
}

#ifdef SARRAY_MAIN

#include <stdio.h>
#include <stdlib.h>  // for malloc and free
#include <ctype.h>   // for isspace

/**
 * @brief Demo walker function that parses words with space delimiters
 *
 * This function implements the `walker_func` function signature
 * for use by @ref sarray_measure and @ref sarray_build.
 */
void custom_string_walker(const char *string, put_string_func func, void *data)
{
   const char *start = string;
   const char *end = string;

   while (*end)
   {
      if (isspace(*end))
      {
         // hand the substring to sarray function
         (*func)(start, end-start, data);

         // skip extra consecutive spaces
         while (isspace(*end))
            ++end;
         // reset for next word
         start = end;
      }

      ++end;
   }

   // Save remaining characters, if any
   if (end > start+1)
      (*func)(start, end-start, data);
}

/**
 * @brief Use the sarray_handle object to print a report.
 */
void display_array(sarray_handle *handle)
{
   const char **ptr = handle->strings;
   const char **end = ptr + handle->count;

   while (ptr < end)
   {
      printf("\"%s\"\n", *ptr);
      ++ptr;
   }
}

/**
 * @brief Demonstrates how to use sarray functions to build an array
 *
 * Assuming the string-walker function is available
 * (@ref custom string_walkter), this function demonstrates how
 * to use the sarray mini library.
 *
 * Steps to use sarray:
 * 1. Create a handle for the result.
 * 2. Measure memory needs with @ref sarray_measure.
 * 3. Allocate a block of memory of the necessary size.
 * 4. Build the array with @ref sarray_build.
 * 5. Use the array through the sarray_handle.
 * 6. (If necessary) free the allocated memory.
 */
void run_demo(const char *str)
{
   sarray_handle sh = { 0 };

   int len = sarray_measure(str, custom_string_walker);
   if (len)
   {
      char *buff = (char*)malloc(len);
      if (buff)
      {
         sarray_build(&sh, str, custom_string_walker, buff, len);

         display_array(&sh);

         free(buff);
      }

   }
   else
      printf("No substrings found in '%s'\n", str);
}

int main(int argc, const char **argv)
{
   if (argc < 2)
   {
      printf("Please provide one or more strings to process.\n");
      return 1;
   }

   // Use pointers to track arg processing progress
   const char **arg = argv;
   const char **end = arg + argc;

   // skip the command name
   ++arg;

   while (arg < end)
   {
      run_demo(*arg);
      ++arg;
   }

   return 0;
}

#endif


/* Local Variables:               */
/* compile-command: "gcc         \*/
/* -Wall -Werror -std=c99 -ggdb  \*/
/* -DSARRAY_MAIN                 \*/
/* -fsanitize=address            \*/
/* -o sarray sarray.c"           \*/
/* End:                           */
