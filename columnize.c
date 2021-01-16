#include <stdio.h>
#include <string.h>

#include <readargs.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "columnize.h"

/*
 * Use simple ioctl function to query the screen size in
 * characters.  Return the dimensions in pointer arguments
 * *wide* and *tall*.
 */
void get_screen_dimensions(int *wide, int *tall)
{
   // Refer to *man* pages 'ioctl' and 'ioctl_tty'
   struct winsize ws;
   ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
   *wide = ws.ws_col;
   *tall = ws.ws_row;
}

/*
 * Return the word count and size of the longest string
 * in pointer arguments *count* and *maxlen* to predict
 * the number of rows and columns can be displayed.
 */
void tabulate_for_columns(const char **start, const char **end, int *count, int *maxlen)
{
   *count = *maxlen = 0;

   const char **ptr = start;
   size_t curlen;
   while (ptr < end)
   {
      curlen = strlen(*ptr);

      ++*count;
      if (*maxlen < curlen)
         *maxlen = curlen;

      ++ptr;
   }
}

/**
 * Flow columnar data top-to-bottom before moving to next column to the right.
 *
 * The function returns a pointer to the string following the last printed string.
 */
const char ** display_newspaper_columns(const char **start,
                                        const char **end,
                                        int gutter,
                                        int max_columns,
                                        int max_lines)
{
   int wide, tall;
   get_screen_dimensions(&wide, &tall);

   int count, maxlen;
   tabulate_for_columns(start, end, &count, &maxlen);

   int colwidth = maxlen + gutter;
   int columns = wide / colwidth;

   if (max_columns && columns > max_columns)
      columns = max_columns;

   int lines = count / columns;
   if (count % columns)
      ++lines;

   // Copy *end* in case a line limit also limits strings to print
   const char **stop = end;

   if (max_lines && max_lines < lines)
   {
      lines = max_lines;
      stop = start + ( lines * columns );
   }

   const char **anchor_line = start;
   const char **ptr = start;

   while (ptr < stop)
   {
      printf("%-*s", colwidth, *ptr);

      // skip ahead to neighboring string
      ptr += lines;

      // If calcluated neighboring string is out-of-range,
      if (ptr >= stop)
      {
         // Return pointer to string that belongs in left-most column,
         ptr = ++anchor_line;
         // then move to the left-most column for the next print.
         printf("\n");

         // We're done if we've gone past the final string on the left column
         if (ptr >= start+lines)
            break;
      }
   }
   
   printf("\n");

   return stop;
}

/**
 * Flow columnar data left-to-right before moving down to next line.
 *
 * The function returns a pointer to the string following the last printed string.
 */
const char ** display_parallel_columns(const char **start,
                                       const char **end,
                                       int gutter,
                                       int max_columns,
                                       int max_lines)
{
   int wide, tall;
   get_screen_dimensions(&wide, &tall);

   int count, maxlen;
   tabulate_for_columns(start, end, &count, &maxlen);

   int colwidth = maxlen + gutter;
   int columns = wide / colwidth;

   if (max_columns && columns > max_columns)
      columns = max_columns;

   const char **ptr = start;

   int lines = count / columns;
   if (count % columns)
      ++lines;

   // Copy *end* in case a line limit also limits strings to print
   const char **stop = end;

   if (max_lines && max_lines < lines)
   {
      lines = max_lines;
      stop = start + ( lines * columns );
   }

   int column = 0;
   while (ptr < stop)
   {
      printf("%-*s", colwidth, *ptr);

      column = ((column+1) % columns);
      if (!column)
         printf("\n");

      ++ptr;
   }

   printf("\n");

   return stop;
}

#ifdef COLUMNIZE_MAIN

#include "arrayify.c"
#include "get_keypress.c"

typedef const char ** (*flow_function_f)(const char **start,
                                         const char **end,
                                         int gutter,
                                         int max_columns,
                                         int max_lines);

/*
 * State variables to be handled with *readargs*
 */
int gutter = 3;
int max_columns = 0;
int show_format_demo = 0;
int show_screen_specs = 0;
int show_paged_output = 0;
int max_lines = 0;
const char **first_string = NULL;
const char *strings_file = NULL;
flow_function_f flow_function = display_newspaper_columns;


/*
 * The following code is included to illustrate the ideas I'm exploring
 * with this code.  The demonstration code will only be compiled when
 * using EMACS `M-x compile` with this being the current buffer, so the
 * files *columnize.h* and *columnize.c* can be included in other
 * projects without polluting those projects with demonstration code.
 */


void use_arrayified_string(int argc, const char **argv, void *closure)
{
   const char **ptr = argv;
   const char **end = argv + argc;

   while (ptr < end)
      ptr = (*flow_function)(ptr, end, gutter, max_columns, max_lines);
}

void demo_string_formatting(void)
{
   /*
    * Refer to `man 3 printf` and search for "Format of the format string"
    *
    * You can print a substring by using the *field width* and *field precision*
    * settings of a %s formatting token.  These are the string behaviors of formatting
    * instructions that may be more familiar for printing float/double values,
    * where the *width* is the entire length of the output, and the *precision*
    * says how many numerals can follow the decimal.
    */

   printf("In the following examples, each example line begins\n"
          "with the conversion specifier that creates the output\n"
          "that follows the specifier.\n" );

   printf("Formatting results of float value 12345.09876.\n");
   printf("%%12.5f: ->%12.5f<-\n", 12345.09876);
   printf("%%.12f:  ->%.12f<-\n", 12345.09876);
   printf("%%5.f:   ->%5.f<-\n", 12345.09876);

   printf("\n");

   printf("Using the same value, use variable field width and\n"
          "precision values.\n");

   printf("%%*.f:  ->%*.f<-\n", 3, 12345.09876);
   printf("%%*.*f: ->%*.*f<-\n", 3, 3, 12345.09876);

   printf("\n");

   printf("Formatting results of string value abcdefghij.\n");
   printf("%%5.5s: ->%5.5s<-\n", "abcdefghij");
   printf("%%.5s:  ->%.5s<-\n", "abcdefghij");
   printf("%%5s:   ->%5s<-\n", "abcdefghij");
   printf("%%5.s:  ->%5.s<-\n", "abcdefghij");
   printf("NOTE the lastline of the output is surprising.\n"
          "Unlike the numeric specifier \"%%5.f\" prints 5 characters,\n"
          "using \"%%5.s\" prints empty spaces instead of the contents\n"
          "of the string, while \"%%5s\" prints the string.\n");

   printf("\n");
   

}

/*
 * Code to implement custom *readargs* agents.
 *
 * A *reader* function is required.  The *reader* function responds
 * to a command line argument by saving it or changing a setting.
 *
 * A *writer* function is optional and is only used by the
 * *readargs* agent `ra_show_values_agent` to inform the user
 * what is the current setting backed by the *raAction* entry.
 */

/*
 * Custom *readargs* agent *flow_agent* will change the
 * value of a function pointer used to display an array
 * of strings.
 */

// Custom readargs agent to set flow function
raStatus flow_reader(const raAction *act, const char *str, raTour *tour)
{
   if (!str)
      return RA_MISSING_VALUE;

   if (*str == 'n')
      *(flow_function_f*)act->target = display_newspaper_columns;
   else if (*str == 'p')
      *(flow_function_f*)act->target = display_parallel_columns;
   
   return RA_SUCCESS;
}

void flow_writer(FILE *f, const raAction *act)
{
   flow_function_f fff = *(flow_function_f*)act->target;
   if (fff == display_newspaper_columns)
      fprintf(f, "newspaper flow");
   else if (fff == display_parallel_columns)
      fprintf(f, "parallel flow");
   else
      fprintf(f, "undefined flow");
}

/* The agent needs the value following the option, thus the
 * first element of the agent is 1.
 */
const raAgent flow_agent = { 1, flow_reader, flow_writer };

/*
 * The following custom *readargs* agent is unusual because
 * it only identifies the first non-option argument that will
 * be used as the first element for the columnar output.
 *
 * The demo program depends on all option arguments preceding
 * the first data argument.  Knowing that, this agent omits a
 * *writer* function because when the *ra_show_values_agent*
 * agent would be triggered, this value will not yet have been
 * determined.
 * 
 * This function is ususual because it uses a triple-pointer
 * to set the value of a double-pointer (pointer to array of
 * string pointers).
 */

// Custom readargs agent to identify the first string
raStatus findarg_reader(const raAction *act, const char *str, raTour *tour)
{
   if (!str)
      return RA_MISSING_VALUE;

   const char ***firstarg = (const char ***)act->target;
   *firstarg = (const char **)tour->current_arg;
   
   return RA_SUCCESS;
}

const raAgent findarg_agent = { 1, findarg_reader, NULL };


void display_columns(int argc, const char **first, void *closure)
{
   const char **end = first + argc;

   // Collect screen dimensions and list specs
   int wide, tall;
   get_screen_dimensions(&wide, &tall);

   int count = 0, maxlen = 0;
   tabulate_for_columns(first, end, &count, &maxlen);

   int columns_to_show = wide / ( maxlen + gutter );
   if ( max_columns && columns_to_show > max_columns )
      columns_to_show = max_columns;

   // Leave two lines for prompt
   int lines_to_show = tall - 2;
   if ( max_lines && lines_to_show > max_lines )
      lines_to_show = max_lines;

   int page_capacity = columns_to_show * lines_to_show;
   int items_in_list = (int)(end - first);
   int page_count = (items_in_list / page_capacity) + 1;
         
   const char **top = first;
         
   while (top < end)
   {
      const char **stop = (*flow_function)(top, end, gutter, columns_to_show, lines_to_show);
      if (show_paged_output)
      {
         printf("\n\nPage %d of %d.  "
                "'n' for next page, "
                "'p' for previous page, "
                "'q' to quit.\n",
                (int)((top - first) / page_capacity) + 1,
                page_count);

         char keybuff[10];
         while (get_keypress(keybuff, sizeof(keybuff)))
         {
            if (*keybuff == 'q')
               goto abandon_display;
            else if (*keybuff == 'n')
            {
               // Only break out if more pages remain
               if (stop < end)
               {
                  top = stop;
                  break;
               }
            }
            else if (*keybuff == 'p')
            {
               // Only break out if previous page exists
               if ((top - first) > page_capacity)
               {
                  top -= page_capacity;
                  break;
               }
            }
         }
      }
      else
         top = stop;
   }

  abandon_display:
   ;
}


raAction actions[] = {
   {'h', "help",        "This help display",                       &ra_show_help_agent },
   {'s', "show_values", "Show set values.",                        &ra_show_values_agent },

   {'c', "columns", "Upper limit of columns to display",           &ra_int_agent,  &max_columns},
   {'F', "flow",    "Flow orientation, (n)ewspaper or (p)arallel", &flow_agent,    &flow_function},
   {'f', "file",    "File with strings to columnize (set IFS to change delimiters)",
              &ra_string_agent, &strings_file},
   {'g', "gutter",  "Minimum spaces between columns",              &ra_int_agent,  &gutter},
   {'l', "lines",   "Line limit per \"page.\"",                    &ra_int_agent,  &max_lines},
   {'r', "rows",   "Row limit per \"page.\"",                      &ra_int_agent,  &max_lines},
   {'d', NULL,      "Show string formatting demo.",                &ra_flag_agent, &show_format_demo},
   {'S', NULL,      "Show screen specs.",                          &ra_flag_agent, &show_screen_specs},
   {'p', "paged",   "Show paged output.",                          &ra_flag_agent, &show_paged_output},
   {-1,  "*list_start", "First string of list",                    &findarg_agent, &first_string}
};
   
int main(int argc, const char **argv)
{
   ra_set_scene(argv, argc, actions, ACTS_COUNT(actions));

   if (ra_process_arguments())
   {
      // Collect screen dimensions and list specs
      int wide, tall;
      get_screen_dimensions(&wide, &tall);

      const char **end = argv + argc;
      int count = 0, maxlen = 0;

      // *first_string* is NULL if the user entered no strings.
      if (first_string)
         tabulate_for_columns(first_string, end, &count, &maxlen);

      if (show_format_demo)
         demo_string_formatting();
      if (show_screen_specs)
      {
         printf("The screen dimensions are %d columns by %d rows.\n",
                wide, tall);

         if (first_string)
            printf("For your list of %d strings, the longest string is %d characters.\n",
                   count, maxlen);
      }

      if (strings_file)
         arrayify_file(strings_file, display_columns, NULL);
      else if (first_string)
         display_columns(end - first_string, first_string, NULL);
   }

   return 0;
}

#endif

/* Local Variables: */
/* compile-command: "b=columnize; \*/
/*  cc -Wall -Werror -ggdb        \*/
/*  -std=c99 -pedantic            \*/
/*  -lreadargs                    \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"   \*/
/* End: */

