#include <stdio.h>
#include <string.h>

#include <readargs.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef const char ** (*flow_function_f)(const char **start,
                                         const char **end,
                                         int gutter,
                                         int max_columns,
                                         int max_lines);


/*
 * There are two styles of columnar output:
 * - "Newspaper style" with consecutive strings are below each other in
 *   each column:
 *
 *   1  4  7
 *   2  5  8
 *   3  6  9
 *
 * - "Parallel style" where consecutive strings are added to the right:
 *
 *   1  2  3
 *   4  5  6
 *   7  8  9
 *
 * In addition to code to print a set of strings in columns, this
 * code also includes:

 * - Explains formatting printf output for a string "%*s", "%.*s"
 *   Refer to function `pad_write_string` and `demo_string_formatting`.
 *
 * - A custom *readargs* agent. Refer to `findarg_reader()`, `findarg_writer()`,
 *   the agent definition following the two functions, and the *list_start raAction
 *   in the action map.
 *
 */


void tabulate(const char **start, const char **end, int *count, int *maxlen)
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

void get_screen_length(int *wide, int *tall)
{
   // Refer to *man* pages 'ioctl' and 'ioctl_tty'
   struct winsize ws;
   ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
   *wide = ws.ws_col;
   *tall = ws.ws_row;
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

   printf("Printing 10 spaces with \"%%*s\" formatting:\n");
   printf("This is using the *width* format instruction.\n");
   printf("->%*s<-\n", 10, "");
   printf("\n");

   printf("Print only the first 5 letters of string \"1234567890\" with \"%%.*s\"\n");
   printf("This is using the *precision* format instruction.\n");
   printf("%.*s\n", 5, "1234567890");

   printf("\n");
   printf("Use these variable string output behaviors to add padding before or\n");
   printf("after a string, as the main columnar output does.\n");
          
   printf("\n");
   printf("This example prints a substring string in a right-justified fixed 10 character cell.\n");
   printf("That means we add padding before the string:\n");
   printf("printf(\"%%*s%%.*s\", 5, \"\", 5, \"1234567890\");\n");
   printf("%*s%.*s\n", 5, "", 5, "1234567890");

   printf("\n");
   printf("This example prints a substring string in a left-justified fixed 10 character cell.\n");
   printf("That means we add padding after the string:\n");
   printf("printf(\"%%.*s%%*s<-\", 5, \"1234567890\", 5, \"\");\n");
   printf("%.*s%*s<-\n", 5, "1234567890", 5, "");
}

void pad_write_string(const char *str, int maxlen)
{
   int curlen = strlen(str);
   printf("%s%*.s", str, maxlen - curlen, "");
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
   get_screen_length(&wide, &tall);

   int count, maxlen;
   tabulate(start, end, &count, &maxlen);

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
      pad_write_string(*ptr, colwidth);

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
   get_screen_length(&wide, &tall);

   int count, maxlen;
   tabulate(start, end, &count, &maxlen);

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
      pad_write_string(*ptr, colwidth);

      column = ((column+1) % columns);
      if (!column)
         printf("\n");

      ++ptr;
   }

   printf("\n");

   return stop;
}

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

const raAgent flow_agent = { 1, flow_reader, flow_writer };

// Custom readargs agent to identify the first string
raStatus findarg_reader(const raAction *act, const char *str, raTour *tour)
{
   if (!str)
      return RA_MISSING_VALUE;

   const char ***firstarg = (const char ***)act->target;
   *firstarg = (const char **)tour->current_arg;
   
   return RA_SUCCESS;
}

// There is not writer function for findarg_agent.
// A writer function will never have a useful value for
// ra_show_values_agent because the first argument necessarily
// follows all program options, the parsing of which might
// trigger ra_show_values_agent.

const raAgent findarg_agent = { 1, findarg_reader, NULL };

int gutter = 3;
int max_columns = 0;
int show_format_demo = 0;
int show_screen_specs = 0;
int max_lines = 0;
const char **first_string = NULL;
flow_function_f flow_function = display_newspaper_columns;

raAction actions[] = {
   {'h', "help", "This help display", &ra_show_help_agent },
   {'c', "columns", "Upper limit of columns to display", &ra_int_agent, &max_columns},
   {'f', "flow", "Flow orientation, (n)ewspaper or (p)arallel", &flow_agent, &flow_function},
   {'g', "gutter", "Minimum spaces between columns", &ra_int_agent, &gutter},
   {'l', "lines", "Line limit per \"page.\"", &ra_int_agent, &max_lines},
   {'s', "show values", "Show set values.", &ra_show_values_agent },
   {'d', NULL, "Show string formatting demo.", &ra_flag_agent, &show_format_demo },
   {'S', NULL, "Show screen specs.", &ra_flag_agent, &show_screen_specs },
   {-1,  "*list_start", "First string of list", &findarg_agent, &first_string }
};
   
int main(int argc, const char **argv)
{
   ra_set_scene(argv, argc, actions, ACTS_COUNT(actions));


   if (ra_process_arguments())
   {
      // Collect screen dimensions and list specs
      int wide, tall;
      get_screen_length(&wide, &tall);

      const char **end = argv + argc;
      int count = 0, maxlen = 0;

      // *first_string* is NULL if the user entered no strings.
      if (first_string)
         tabulate(first_string, end, &count, &maxlen);

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

      if (first_string)
      {
         const char **top = first_string;
         while (top < end)
         {
            const char **stop = (*flow_function)(top, end, gutter, max_columns, max_lines);
            top = stop;
         }
      }

      /* display_parallel_columns(first_string end, gutter, max_columns); */
      /* display_newspaper_columns(first_string, end, gutter, max_columns); */
   }

   return 0;
}


/* Local Variables: */
/* compile-command: "b=columnize; \*/
/*  cc -Wall -Werror -ggdb        \*/
/*  -std=c99 -pedantic            \*/
/*  -lreadargs                    \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"   \*/
/* End: */
