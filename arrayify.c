#include "arrayify.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>

const char *arrayify_ifs = " \t\n";

void arrayify_set_ifs_from_env(void)
{
   char *env = getenv("IFS");
   if (env)
      arrayify_ifs = env;
}

int arrayify_is_ifs(int chr)
{
   return strchr(arrayify_ifs, chr) != NULL;
}

char arrayify_escape_chars[] = {
   '\n',
   '\t'
};

const char escapes[] = "nt";

/*
 * This function responds to a backslash-escaped character, and returns
 * indication if the escaped character is a field separator.
 *
 * The function updates pointers to *source* and, if passed in, the
 * sets the *target* value and updates its pointer as well.
 */
int arrayify_process_escaped_char(char **target, const char **source)
{
   if (**source == '\\')
   {
      // Skip backslash that introduces escaped character
      ++*source;

      // Recognize if the following character is IFS before it's translated.
      int is_ifs = arrayify_is_ifs(**source);

      // Attempt to recognize following character
      char *chr = strchr(escapes, **source);
      char translated_chr = chr ? arrayify_escape_chars[chr - escapes] : **source;

      // Only copy char if target is available
      if (*target)
      {
         **target = translated_chr;
         ++*target;
      }

      // Now skip the escaped character
      ++*source;

      // If escaped character is IFS, we recognize as NON-IFS:
      if (is_ifs)
         return 0;
      else if (arrayify_is_ifs(translated_chr))
         return 1;
   }

   return 0;
}

int arrayify_parser(char *buffer, int bufflen, const char **els, int elslen)
{
   int count = 0;
   char *ptr = buffer;
   const char *end = buffer + bufflen;

   const char **cur_el = els;
   const char **end_el = els + elslen;

   // ignore leading spaces
   while (arrayify_is_ifs(*ptr) && ptr < end)
      ++ptr;

   char *target = els ? ptr : NULL;
   const char *source = ptr;

   if (target)
      *cur_el = ptr;

   while (source < end)
   {
      int advance_element = 0;

      // For counting, just ignore characters following a back-slash.
      if (*source == '\\')
         advance_element = arrayify_process_escaped_char(&target, &source);
      else if (arrayify_is_ifs(*source))
      {
         ++source;
         ++count;

         if (target)
            *target++ = '\0';

         // discard all spaces between elements
         while (arrayify_is_ifs(*source) && source < end)
            ++source;

         if (source < end)
         {
            if (target)
            {
               ++cur_el;
               if (cur_el >= end_el)
               {
                  printf("There should be %d elements in the els array.\n", elslen);
                  printf("Diff between cur_el and els is %ld.\n", cur_el - els);
                  printf("Diff between cur_el and end_el is %ld.\n", cur_el - end_el);
               }
               assert(cur_el < end_el);
               *cur_el = source;

               target = (char*)source;
            }
         }
      }
      else if (target)
         *target++ = *source++;
      else
         source++;

      if (advance_element)
      {
         ++cur_el;
      }
   }

   return count;
}


void arrayify_string(char *buffer, int bufflen, arrayify_user user)
{
   int count = arrayify_parser(buffer, bufflen, NULL, 0);
   const char *els[count];
   memset(els, 0, sizeof(els));

   arrayify_parser(buffer, bufflen, els, count);

   (*user)(count, (const char**)els);
}



#ifdef ARRAYIFY_MAIN

#include <stdio.h>
#include <readargs.h>
#include <fcntl.h>     // for open(), stat()
#include <sys/stat.h>  // for open(), stat()

#include <unistd.h>   // for read()
#include <alloca.h>

#include <errno.h>

const char *filepath = NULL;
int show_ifs_flag = 0;

raAction actions[] = {
   {'h', "help", "This help display", &ra_show_help_agent },
   {-1, "*file", "File to parse", &ra_string_agent, &filepath },
   {'I', "show-ifs", "Display IFS", &ra_flag_agent, &show_ifs_flag },
   {'i', "set-ifs", "Set IFS", &ra_string_agent, &arrayify_ifs }
};

void show_ifs(void)
{
   const char *ptr = arrayify_ifs;
   int count = 0;
   while(*ptr)
   {
      printf("%d: %d\n", ++count, *ptr);
      ++ptr;
   }
}

int alt_main(int argc, const char **argv)
{
   printf("There are %d elements in the array.\n", argc);
   const char **ptr = argv;
   const char **end = argv + argc;
   int index = 0;

   while (ptr < end)
   {
      printf("%4d: %s\n", index++, *ptr);
      ++ptr;
   }
   return 0;
}

int read_file(const char *path)
{
   struct stat lstat;
   if (stat(path, &lstat) == 0)
   {
      unsigned size = lstat.st_size;
      if (size > 0)
      {
         int fh = open(path, O_RDONLY);
         if (fh)
         {
            char *buffer = (char*)alloca(size+1);
            unsigned bytes_read = read(fh, buffer, size+1);
            if (bytes_read < size+1)
               buffer[bytes_read] = '\0';

            // Don't include terminating \0 in bytes_read count:
            arrayify_string(buffer, bytes_read, alt_main);
         }
      }
   }
   else
      fprintf(stderr, "stat on %s failed: %s.\n", path, strerror(errno));

   return errno;
}

int main(int argc, const char **argv)
{
   ra_set_scene(argv, argc, actions, ACTS_COUNT(actions));

   if (ra_process_arguments())
   {
      if (show_ifs_flag)
         show_ifs();
      
      if (filepath)
         read_file(filepath);
      else
         printf("Nothing to do.\n");
   }
   return 0;
}


/* Local Variables: */
/* compile-command: "b=arrayify; \*/
/*  cc -Wall -Werror -ggdb        \*/
/*  -std=c99 -pedantic            \*/
/*  -lreadargs                    \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"   \*/
/* End: */

#endif
