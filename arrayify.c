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

char arrayify_convert_escaped_char(char c)
{
   char *chr = strchr(escapes, c);
   if (chr)
      return arrayify_escape_chars[chr - escapes];
   else
      return c;
}

/*
 * Skip past unescaped or escape-identified IFS characters.
 * Returns *end* for no normal characters to preserve termination
 * conditions in arrayify_parser() loop.
 */
const char *arrayify_trim_ifs(const char *buffer, const char *end)
{
   const char *ptr = buffer;
   while (ptr < end)
   {
      if (*ptr == '\\')
      {
         char c = arrayify_convert_escaped_char(*(ptr+1));
         if (!arrayify_is_ifs(c))
            return ptr;  // return pointer to the backslash
         else
            ++ptr;       // increment past the backslash
      }
      else if (!arrayify_is_ifs(*ptr))
         return ptr;

      ++ptr;
   }

   return end;
}

int arrayify_parser(char *buffer, int bufflen, const char **els, int elslen)
{
   // Guardrail for buffer
   char *end = buffer + bufflen;

   const char *source = arrayify_trim_ifs(buffer, end);

   // Early termination for empty string
   if (source >= end)
      return 0;

   // At this point, we have at least one element.
   int count = 1;

   const char **cur_el = els;
   const char **end_el = els + elslen;

   // *target* will be used as flag to enable copying
   char *target = els ? buffer : NULL;
   if (target)
      *cur_el = target;

   char prepped_char = 0;

   while (source < end)
   {
      if (*source == '\\')
      {
         ++source;

         // (A bit confusing: think about it)
         // If escaped character is IFS, the escape was intended
         // to disarm IFS, the keep in current string.
         if (arrayify_is_ifs(*source))
            prepped_char = *source;
         else
         {
            prepped_char = arrayify_convert_escaped_char(*source);

            // If converted character is IFS, then it was intended
            // to be interpreted as IFS, mark as end-of-element
            if (arrayify_is_ifs(prepped_char))
               prepped_char = '\0';
         }
      }
      else if (arrayify_is_ifs(*source))
         prepped_char = '\0';
      else
         prepped_char = *source;

      ++source;

      if (target)
      {
         *target = prepped_char;
         ++target;
      }

      // Signal to end the current element:
      if (prepped_char == '\0')
      {
         source = arrayify_trim_ifs(source, end);

         if (source < end)
         {
            ++count;

            if (els)
            {
               ++cur_el;

               if (cur_el < end_el)
                  *cur_el = target;
               else
               {
                  fprintf(stderr, "Attempted arrayify buffer-overrun.\n");
                  break;
               }
            }
         }
         else
            break;

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
            char buffer[size+1];
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
