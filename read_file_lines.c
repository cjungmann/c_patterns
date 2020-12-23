
#include <string.h>    // for memcpy()

#include <sys/types.h> // for open()
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>    // for read()
#include <errno.h>     // for strerror()


unsigned bufflen = 2048;

typedef int bool;

typedef bool (*line_user)(const char *start, const char *end, void *closure);

/**
 * Returns 0 for success, the errno value if it fails.
 */
int read_file_lines(int fh, line_user user, void *closure)
{
   char buffer[bufflen];
   char *buffend = buffer + sizeof(buffer);

   char *start_read = buffer;   // memory position at which next read begins
   char *end_read;              // points to memory position limit of most recent read

   char *start_line = NULL;     // beginning of the next-to-be-saved line
   char *end_line = NULL;       // end of the next-to-be-saved line
   char *next_line = NULL;      // position to copy to start_line after the line is saved

   char *ptr;                   // pointer to most-recently considered char

   size_t bytes_read;

   while (0 < (bytes_read = read(fh, start_read, buffend-start_read)))
   {
      // clear left-over values from last processed line
      end_line = next_line = NULL;

      if (bytes_read == -1)
         return errno;
      else if (bytes_read == 0)
      {
         end_line = start_read;
         break;
      }

      // All loops should start at the beginning of a line
      start_line = ptr = buffer;

      // Keep track of the end of the data
      end_read = start_read + bytes_read;

      while (ptr < end_read)
      {
         if (*ptr == '\r')
         {
            end_line = ptr;
            ++ptr;

            if (*ptr == '\n')
               next_line = ++ptr;
            else
               next_line = ptr;
         }
         else if (*ptr == '\n')
         {
            end_line = ptr;
            next_line = ++ptr;
         }
         else
            ++ptr;

         if (end_line)
         {
            if (!(*user)(start_line, end_line, closure) || ptr >= end_read)
               goto abandon_function;
            else
            {
               start_line = next_line;
               end_line = NULL;
            }
         }
      }

      // Prepare pointers for loop to next read():

      // measure length of the incomplete line
      int move_bytes = end_read - start_line;
      // copy incomplete line beginning of the buffer
      memcpy(buffer, start_line, move_bytes);
      // prepare to read file to byte following incomplete line
      start_read = buffer + move_bytes;

      end_line = buffer + move_bytes;
   }

   if (end_line > buffer)
      (*user)(buffer, end_line, closure);

  abandon_function:
   return 0;
}


#ifdef READ_FILE_LINES_MAIN

// These includes only necessary for this specific implementation:
#include <stdio.h>
#include "commaize.c"

#include <ctype.h>     // for isspace()
#include <stdint.h>    // for uint64_t type
#include <stdlib.h>    // for atol()

#include <readargs.h> 

/**
 * Read line backwards, so everything before the last set of spaces
 * is the string associated with the line-ending number.
 */
void interpret_string_number(const char /*in*/ *start, const char /*in*/ *end,
                             const char /*out*/ **word_end, uint64_t /*out*/ *number)
{
   const char *ptr = end-1;

   while (isdigit(*ptr))
      --ptr;

   // copy number in case, on the last iteration,
   // the character following *end is another numeral
   int numlen = end - ptr - 1;
   char numstr[numlen+1];
   memcpy(numstr, ptr+1, numlen);
   numstr[numlen] = '\0';

   *number = atol(numstr);

   while (isspace(*ptr))
      --ptr;

   // The word-ending space ends the word
   *word_end = ptr+1;
}

/**
 * Callback function for test case.
 */
bool read_file_lines_user(const char *start, const char *end, void *closure)
{
   const char *end_word;
   uint64_t   value;
   interpret_string_number(start, end, &end_word, &value);

   printf("\"%.*s\" refers to number ", (int)(end_word - start), start);
   commaize_number(value);
   printf(".\n");

   return 1;
}

const char *filename = "read_file_lines.test";

raAction actions[] = {
   {'h', "help", "Help screen", &ra_show_help_agent },
   {'b', "bufflen", "Length of line-reading buffer", &ra_int_agent, &bufflen },
   {'f', "file", "File to read", &ra_string_agent, &filename }
};

int main(int argc, const char **argv)
{
   int errnum = 0;

   ra_set_scene(argv, argc, actions, ACTS_COUNT(actions));

   if (ra_process_arguments())
   {
      int fh = open(filename, O_RDONLY);
      if (fh > 0)
      {
         errnum = read_file_lines(fh, read_file_lines_user, NULL);
         close(fh);

         if (errnum)
            printf("read_file_lines failed with \"%s\".\n", strerror(errno));
      }
      else
         printf("Failed to open \"%s\".\n", filename);
   }

   return errnum;
}

#endif


/* Local Variables: */
/* compile-command: "b=read_file_lines; \*/
/*  cc -Wall -Werror -ggdb        \*/
/*  -std=c99 -pedantic            \*/
/*  -lreadargs                    \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"   \*/
/* End: */
