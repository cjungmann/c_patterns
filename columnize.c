#include <stdio.h>
#include <string.h>

#include <readargs.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "columnize.h"
#include "get_keypress.h"


int columnize_string_get_len(const void *el)
{
   return strlen((const char *)el);
}

int columnize_string_print(FILE *f, const void *el)
{
   return fprintf(f, "%s", (const char*)el);
}

int columnize_string_print_cell(FILE *f, const void *el, int width)
{
   return fprintf(f, "%-*s", width, (const char*)el);
}

CEIF ceif_string = {
   columnize_string_get_len,
   columnize_string_print,
   columnize_string_print_cell
};

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
void tabulate_for_columns(const char **start, const char **end, int *maxlen)
{
   *maxlen = 0;

   const char **ptr = start;
   size_t curlen;
   while (ptr < end)
   {
      curlen = strlen(*ptr);

      if (*maxlen < curlen)
         *maxlen = curlen;

      ++ptr;
   }
}

int columnize_get_max_len(CEIF *iface, const void **start, const void **end)
{
   int cur_len, max_len = 0;
   for (const void **ptr = start; ptr < end; ++ptr)
   {
      cur_len = (*iface->get_len)(*ptr);
      if (cur_len > max_len)
         max_len = cur_len;
   }
   return max_len;
}

/**
 * Flow columnar data top-to-bottom before moving to next column to the right.
 *
 * The function returns a pointer to the string following the last printed string.
 */
const void ** display_newspaper_columns(CEIF *iface,
                                        const void **start,
                                        const void **end,
                                        int gutter,
                                        int max_columns,
                                        int max_lines)
{
   int wide, tall;
   get_screen_dimensions(&wide, &tall);

   int count = (int)(end - start);
   int maxlen = columnize_get_max_len(iface, start, end);

   int colwidth = maxlen + gutter;
   int columns = wide / colwidth;

   if (max_columns && columns > max_columns)
      columns = max_columns;

   int page_capacity = columns * (max_lines ? max_lines : tall);

   // Adjust stopping point if restricted by caller
   // preferences or page capacity
   const void **stop = end;
   if (max_lines)
   {
      if (count < page_capacity)
         stop = start + count;
      else
         stop = start + page_capacity;
   }

   int lines;

   if (max_lines)
      lines = max_lines;
   else
   {
      lines = count / columns;
      // Extra line needed if not evenly divisible
      if (count % columns)
         ++lines;
   }

   const void **anchor_line = start;
   const void **ptr = start;

   int counter = 0;

   while (ptr < stop)
   {
      ++counter;
      
      (*iface->print_cell)(stdout, *ptr, colwidth);
      // skip ahead to neighboring string
      ptr += lines;

      // If calcluated neighboring string is out-of-range,
      if (ptr >= stop)
      {
         // return pointer to string that belongs in left-most column,
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
const void ** display_parallel_columns(CEIF *iface,
                                       const void **start,
                                       const void **end,
                                       int gutter,
                                       int max_columns,
                                       int max_lines)
{
   int count = end - start;

   int wide, tall;
   get_screen_dimensions(&wide, &tall);

   int maxlen = columnize_get_max_len(iface, start, end);

   int colwidth = maxlen + gutter;
   int columns = wide / colwidth;

   if (max_columns && columns > max_columns)
      columns = max_columns;

   const void **ptr = start;

   int lines = count / columns;
   if (count % columns)
      ++lines;

   // Adjust stopping point if restricted by caller
   // preferences or page capacity
   const void **stop = end;
   if (max_lines && max_lines < lines)
   {
      lines = max_lines;
      stop = start + ( lines * columns );
   }

   int column = 0;
   while (ptr < stop)
   {
      (*iface->print_cell)(stdout, *ptr, colwidth);

      column = ((column+1) % columns);
      if (!column)
         printf("\n");

      ++ptr;
   }

   printf("\n");

   return stop;
}

/*
 * Easy-set best dimension settings.
 */
void columnize_default_dims(struct columnize_page_dims *dims)
{
   dims->flower = display_newspaper_columns;
   dims->pcontrol = columnize_default_controller;
   dims->gutter = 3;
   dims->max_columns = 0;
   dims->reserve_lines = 2;
   dims->paged_output = 1;
}

/*
 * Paged presentation of columnar data, invoking a callback function
 * for moving between pages or to quit.
 */
void columnize_pager(CEIF *iface,
                     const void **elements,
                     int element_count,
                     struct columnize_page_dims *dims)
{
   const void **end = elements + element_count;

   // Collect screen dimensions and list specs
   int wide, tall;
   get_screen_dimensions(&wide, &tall);

   int max_el_length = columnize_get_max_len(iface, elements, end);

   int columns_to_show = wide / ( max_el_length + dims->gutter );
   if ( dims->max_columns && columns_to_show > dims->max_columns )
      columns_to_show = dims->max_columns;

   // Leave two lines for prompt
   int lines_to_show = tall - dims->reserve_lines;

   int page_capacity = columns_to_show * lines_to_show;
   int page_count = (element_count / page_capacity) + 1;
         
   if (!dims->paged_output || page_count == 1)
   {
      // Don't set max_lines to produce unpaged output.
      (*dims->flower)(iface, elements, end, dims->gutter, columns_to_show, 0);
   }
   else
   {
      const void **top = elements;

      while (top < end)
      {
         // print data
         const void **stop = (*dims->flower)(iface,
                                             top,
                                             end,
                                             dims->gutter,
                                             columns_to_show,
                                             lines_to_show);

         int curpage = (int)(top - elements) / page_capacity + 1;

         int dir = (*dims->pcontrol)(curpage, page_count);
         switch(dir)
         {
            case CPR_QUIT:
               goto abandon_keywait;
               break;

            case CPR_NEXT:
               if (stop < end)
                  top = stop;
               break;

            case CPR_PREVIOUS:
               if (curpage > 1)
                  top -= page_capacity;
               break;

            case CPR_FIRST:
               top = elements;
               break;

            case CPR_LAST:
               top = elements + ((page_count-1) * page_capacity);
               break;
         }

      }
     abandon_keywait:
      printf("\x1b[2K");
   }
}

void columnize_string_pager(const char **elements,
                            int element_count,
                            struct columnize_page_dims *dims)
{
   const void **start = (const void**)elements;
   columnize_pager(&ceif_string, start, element_count, dims);
}


CPRD columnize_default_controller(int page_current, int page_count)
{
   CPRD rval = CPR_NO_RESPONSE;
   static const char color_on[] = "\x1b[32;1m";
   static const char color_off[] = "\x1b[m";

   // user chooses next action
   printf("(%2d/%2d) "
          "%sf%sirst "
          "%sn%sext "
          "%sp%srevious "
          "%sl%sast "
          "%sq%suit ",
          page_current, page_count,
          color_on, color_off,
          color_on, color_off,
          color_on, color_off,
          color_on, color_off,
          color_on, color_off
      );

   // Push output without newline
   fflush(stdout);
   // Set cursor to column 1 of current line
   printf("\x1b[1G");

   char keybuff[10];
   while (rval==CPR_NO_RESPONSE && get_keypress(keybuff, sizeof(keybuff)))
   {
      switch(*keybuff)
      {
         case 'q': rval = CPR_QUIT;     break;
         case 'f': rval = CPR_FIRST;    break;
         case 'n': rval = CPR_NEXT;     break;
         case 'p': rval = CPR_PREVIOUS; break;
         case 'l': rval = CPR_LAST;     break;
      }
   }
   
   // Erase page and move cursor to left-most column
   if (rval != CPR_QUIT)
      printf("\x1b[2J\x1b[1G");

   return rval;
}


#ifdef COLUMNIZE_MAIN

#include "arrayify.c"
#include "get_keypress.c"


/*
 * State variables to be handled with *readargs*
 */
struct columnize_page_dims g_dims = { 0 };

int show_format_demo = 0;
int show_screen_specs = 0;
int max_lines = 0;
const char *parsing_ifs = "\n";
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
   const void **ptr = (const void**)argv;
   const void **end = ptr + argc;

   while (ptr < end)
      ptr = (*flow_function)(&ceif_string, ptr, end, g_dims.gutter, g_dims.max_columns, max_lines);
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




void use_string_array(int argc, const char **argv, void *closure)
{
   columnize_string_pager(argv, argc, &g_dims);
}


raAction actions[] = {
   {'h', "help",        "This help display",                     &ra_show_help_agent },
   {'s', "show_values", "Show set values.",                      &ra_show_values_agent },

   // Access to pager dimensions gutter, max_columns, and max_lines
   {'c', "columns", "Upper limit of columns to display",         &ra_int_agent,  &g_dims.max_columns},
   {'g', "gutter",  "Minimum spaces between columns",            &ra_int_agent,  &g_dims.gutter},
   // the following two intentionally change the same variable
   {'l', "lines",   "Line limit per \"page.\"",                  &ra_int_agent,  &max_lines},
   {'r', "rows",    "Row limit per \"page.\"",                   &ra_int_agent,  &max_lines},
   {'p', "paged",   "Show paged output.",                        &ra_flag_agent, &g_dims.paged_output},

   {'F', "flow",    "Flow orientation, (n)ewspaper or (p)arallel", &flow_agent,   &g_dims.flower},
   {'f', "file",    "File with strings to columnize (set IFS to change delimiters)",
              &ra_string_agent, &strings_file},
   {'i', "ifs",     "Internal field (element) separator",        &ra_string_agent, &parsing_ifs},


   {'d', NULL,      "Show string formatting demo.",              &ra_flag_agent, &show_format_demo},
   {'S', NULL,      "Show screen specs.",                        &ra_flag_agent, &show_screen_specs},
   {-1,  "*list_start", "First string of list",                  &findarg_agent, &first_string}
};
   
int main(int argc, const char **argv)
{
   columnize_default_dims(&g_dims);

   arrayify_set_ifs(parsing_ifs);

   ra_set_scene(argv, argc, actions, ACTS_COUNT(actions));

   if (ra_process_arguments())
   {
      // Collect screen dimensions and list specs
      int wide, tall;
      get_screen_dimensions(&wide, &tall);

      const char **end = argv + argc;
      int maxlen = 0;

      // *first_string* is NULL if the user entered no strings.
      if (first_string)
         tabulate_for_columns(first_string, end, &maxlen);

      if (show_format_demo)
         demo_string_formatting();
      if (show_screen_specs)
      {
         printf("The screen dimensions are %d columns by %d rows.\n",
                wide, tall);

         if (first_string)
            printf("For your list of %d strings, the longest string is %d characters.\n",
                   (int)(end - first_string), maxlen);
      }

      if (strings_file)
         arrayify_file(strings_file, use_string_array, NULL);
      else if (first_string)
         columnize_string_pager(first_string, end - first_string, &g_dims);
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

