#include <stdio.h>
#include <string.h>

#include <readargs.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "columnize.h"
#include "get_keypress.h"
#include "prompter.h"


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
/* int get_max_size(CEIF *ceif, const void **start, const void **end) */
/* { */
/*    int (*get_len)(const void *el) = ceif->get_len; */

/*    int maxlen = 0; */

/*    void char **ptr = start; */
/*    size_t curlen; */
/*    while (ptr < end) */
/*    { */
/*       curlen = (*get_len)(*ptr); */

/*       if (maxlen < curlen) */
/*          maxlen = curlen; */

/*       ++ptr; */
/*    } */

/*    return maxlen; */
/* } */

int columnize_get_max_len(const CEIF *iface, const void **start, const void **end)
{
   int (*get_len)(const void *el) = iface->get_len;

   int cur_len, max_len = 0;
   for (const void **ptr = start; ptr < end; ++ptr)
   {
      cur_len = (*get_len)(*ptr);
      if (cur_len > max_len)
         max_len = cur_len;
   }
   return max_len;
}

int get_max_string_len(const char **start, const char **end)
{
   return columnize_get_max_len(&ceif_string, (const void**)start, (const void**)end);
}


/**
 * Flow columnar data top-to-bottom before moving to next column to the right.
 * It is an implementation of *flow_function_f* 
 *
 * The function returns a pointer to the string following the last printed string.
 */
const void ** display_newspaper_columns(const CEIF *iface,
                                        const void **start,
                                        const void **end,
                                        int gutter,
                                        int max_columns,
                                        int max_lines)
{
   int count = (int)(end - start);

   int wide, tall;
   get_screen_dimensions(&wide, &tall);

   int maxlen = columnize_get_max_len(iface, start, end);

   int colwidth = maxlen + gutter;
   int columns = wide / colwidth;

   if (max_columns && columns > max_columns)
      columns = max_columns;

   // Adjust stopping point if restricted by caller
   // preferences or page capacity
   const void **stop = end;

   if (max_lines)
   {
      int page_capacity = columns * (max_lines ? max_lines : tall);

      if (count < page_capacity)
         stop = start + count;
      else
         stop = start + page_capacity;
   }

   int items_in_column;

   if (max_lines)
      items_in_column = max_lines;
   else
   {
      items_in_column = count / columns;
      // Extra line needed if not evenly divisible
      if (count % columns)
         ++items_in_column;
   }

   int line_counter = 0;

   const void **anchor_line = start;
   const void **ptr = start;

   while (ptr < stop)
   {
      (*iface->print_cell)(stdout, *ptr, colwidth);

      // skip ahead to item in adjacent column
      ptr += items_in_column;

      // If calcluated neighboring string is out-of-range,
      if (ptr >= stop)
      {
         ++line_counter;

         // return pointer to string that belongs in left-most column,
         ptr = ++anchor_line;
         // then move to the left-most column for the next print.
         printf("\n");

         // We're done if we've gone past the final string on the left column
         if (ptr >= start + items_in_column)
            break;
      }
   }

   // If not enough entries, add extra lines to push first element to top-of-screen:
   while (line_counter++ < max_lines)
      printf("\n");
   
   printf("\n");

   return stop;
}

/**
 * Flow columnar data left-to-right before moving down to next line.
 *
 * The function returns a pointer to the string following the last printed string.
 */
const void ** display_parallel_columns(const CEIF *iface,
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

   // Adjust stopping point if restricted by caller
   // preferences or page capacity
   const void **stop = end;

   int lines = count / columns;
   if (count % columns)
      ++lines;

   if (max_lines && max_lines < lines)
   {
      lines = max_lines;
      stop = start + ( lines * columns );
   }

   int line_counter = 0;
   int column = 0;
   
   const void **ptr = start;

   while (ptr < stop)
   {
      (*iface->print_cell)(stdout, *ptr, colwidth);

      column = ((column+1) % columns);
      if (!column)
      {
         ++line_counter;

         printf("\n");
         /* printf("<%d>\n", line_counter); */
      }

      ++ptr;
   }

   // If not enough entries, add extra lines to push first element to top-of-screen:
   while (line_counter++ < max_lines)
      printf("\n");
      /* printf("-->%d<--\n", line_counter); */

   printf("\n");

   return stop;
}

/*
 * The PPARAM structure holds values that control the
 * columnar output and identify progress through the list.
 *
 * The following functions work with a PPARAM instance to
 * initialize, query, and use the information it tracks.
 */
void PPARAMS_init(PPARAMS *params,
                  const void **start,
                  int el_count,
                  int gutter,
                  int reserve_lines,
                  int max_size)
{
   memset(params, 0, sizeof(PPARAMS));
   
   params->start = start;
   params->end = start + el_count;

   params->gutter = gutter;
   params->reserve_lines = reserve_lines;
   params->max_size = max_size;
}

void PPARAMS_query_screen(PPARAMS *params)
{
   get_screen_dimensions(&params->win_wide, &params->win_tall);
   params->columns_to_show = params->win_wide / (params->max_size + params->gutter);
   params->lines_to_show = params->win_tall - params->reserve_lines;
   params->page_capacity = params->lines_to_show * params->columns_to_show;
}

const void** PPARAMS_first(PPARAMS *params)
{
   params->ptr = params->start;
   return params->ptr;
}

const void** PPARAMS_previous(PPARAMS *params)
{
   const void **newptr = params->ptr - params->page_capacity;
   if (newptr >= params->start)
      params->ptr = newptr;

   return params->ptr;
}

const void** PPARAMS_next(PPARAMS *params)
{
   const void **newptr = params->ptr + params->page_capacity;
   if (newptr < params->end)
      params->ptr = newptr;

   return params->ptr;
}

const void** PPARAMS_last(PPARAMS *params)
{
   int el_count = params->end - params->start;

   // One MIGHT want to factor out page_capacity, but this
   // works because integer division returns the integer
   // floor of the quotient.
   params->ptr = params->start + (params->page_capacity * (el_count / params->page_capacity));

   return params->ptr;
}

int PPARAMS_page_count(const PPARAMS *params)
{
   return (params->end - params->start) / params->page_capacity + 1;
}

/* 0-based index, i.e. the first page is page 0. */
int PPARAMS_current_page(const PPARAMS *params)
{
   return (params->ptr - params->start) / params->page_capacity;
}

int PPARAMS_pointer_index(const PPARAMS *params, const void **ptr)
{
   return (int)(ptr - params->start);
}

PPARAMS_mover pparams_movers[] = {
   PPARAMS_first,
   PPARAMS_previous,
   PPARAMS_next,
   PPARAMS_last
};

const void** PPARAMS_move(PPARAMS *params, CPRD request)
{
   if (request >= CPR_FIRST && request <= CPR_LAST)
      return (*pparams_movers[request - CPR_FIRST])(params);
   else
      return params->ptr;
}

void columnize_print_progress(PPARAMS *params, const void **stop)
{
   int starting_index = PPARAMS_pointer_index(params, params->ptr);
   int ending_index = PPARAMS_pointer_index(params, stop);

   printf("Page %d of %d (items %d to %d)",
          PPARAMS_current_page(params) + 1,
          PPARAMS_page_count(params),
          starting_index+1,
          ending_index);
}

void columnize_print_progress_line(PPARAMS *params, const void **stop)
{
   columnize_print_progress(params, stop);
   printf("\n");
}

/*
 * The following code is included to illustrate the ideas I'm exploring
 * with this code.  The demonstration code will only be compiled when
 * using EMACS `M-x compile` with this being the current buffer, so the
 * files *columnize.h* and *columnize.c* can be included in other
 * projects without polluting those projects with demonstration code.
 */

#ifdef COLUMNIZE_MAIN

#include "arrayify.c"
#include "get_keypress.c"
#include "prompter.c"


int show_format_demo = 0;
int show_screen_specs = 0;
int max_lines = 0;
const char *parsing_ifs = "\n";
const char **first_string = NULL;
const char *strings_file = NULL;
flow_function_f flow_function = display_newspaper_columns;


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

   printf("\n"
          "%%10.5s:  ->%10.5s<-\n"
          "%%-10.5s: ->%-10.5s<-\n",
          "abcdefghij",
          "abcdefghij"
      );

   printf("\n");
   

}

/*
 * Code to implement custom *readargs* agents.
 *
 * First of two readargs custom agents.
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
      flow_function = display_newspaper_columns;
   else if (*str == 'p')
      flow_function = display_parallel_columns;
   
   return RA_SUCCESS;
}

void flow_writer(FILE *f, const raAction *act)
{
   if (flow_function == display_newspaper_columns)
      fprintf(f, "newspaper flow");
   else if (flow_function == display_parallel_columns)
      fprintf(f, "parallel flow");
   else
      fprintf(f, "undefined flow");
}

/* The agent needs the value following the option, thus the
 * first element of the agent is 1.
 */
const raAgent flow_agent = { 1, flow_reader, flow_writer };

/*
 * Second of two readargs agents.
 * 
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

const char *legend_keys[] = {
   "&first",
   "&previous",
   "&next",
   "&last",
   "&quit"
};

int legend_keys_count = sizeof(legend_keys) / sizeof(legend_keys[0]);

// Transform returned index of prompt to CPDR enumeration:
CPRD await_legend_keypress(const char **legend_strings, int count)
{
   int result = prompter_await_prompt(legend_strings, count);
   if (result == 4)
      return CPR_QUIT;
   else
      return result + 1;
}

void columnize_string_array(const char **list, int list_count)
{
   int maxlen = get_max_string_len(list, list + list_count);

   PPARAMS params;
   PPARAMS_init(&params,
                (const void**)list, list_count,
                2,              // gutter size
                3,              // reserve lines (for prompt under columns)
                maxlen);

   PPARAMS_query_screen(&params);

   CPRD cprd;

   // Prepare for initial display at first element
   const void **ptr = PPARAMS_first(&params);

   while (1)
   {
      const void **stop = (*flow_function)(&ceif_string,
                                           ptr,
                                           params.end,
                                           params.gutter,
                                           params.columns_to_show,
                                           params.lines_to_show);

      columnize_print_progress_line(&params, stop);
      prompter_print_prompts(legend_keys, legend_keys_count);

     recheck_user_response:
      cprd = await_legend_keypress((const char **)legend_keys, legend_keys_count);
      if (cprd == CPR_QUIT)
         break;
      else
      {
         const void **newptr = PPARAMS_move(&params, cprd);
         if (newptr != ptr)
            ptr = newptr;
         else
            goto recheck_user_response;
      }
   }

   prompter_reuse_line();
}


void use_arrayified_string(int argc, const char **argv, void *closure)
{
   columnize_string_array(argv, argc);
}

/*
 * arrayify_file() callback function that calls columnize.
 */
void use_string_array(int argc, const char **argv, void *closure)
{
   columnize_string_array(argv, argc);
}

int req_columns = 0;
int req_gutter = 0;
int paged_output = 0;



raAction actions[] = {
   {'h', "help",        "This help display",                     &ra_show_help_agent },
   {'s', "show_values", "Show set values.",                      &ra_show_values_agent },

   // Access to pager dimensions gutter, max_columns, and max_lines
   {'c', "columns", "Upper limit of columns to display",         &ra_int_agent,  &req_columns},
   {'g', "gutter",  "Minimum spaces between columns",            &ra_int_agent,  &req_gutter},
   // the following two intentionally change the same variable
   {'l', "lines",   "Line limit per \"page.\"",                  &ra_int_agent,  &max_lines},
   {'r', "rows",    "Row limit per \"page.\"",                   &ra_int_agent,  &max_lines},
   {'p', "paged",   "Show paged output.",                        &ra_flag_agent, &paged_output},

   {'F', "flow",    "Flow orientation, (n)ewspaper or (p)arallel", &flow_agent },
   {'f', "file",    "File with strings to columnize (set IFS to change delimiters)",
              &ra_string_agent, &strings_file},
   {'i', "ifs",     "Internal field (element) separator",        &ra_string_agent, &parsing_ifs},


   {'d', NULL,      "Show string formatting demo.",              &ra_flag_agent, &show_format_demo},
   {'S', NULL,      "Show screen specs.",                        &ra_flag_agent, &show_screen_specs},
   {-1,  "*list_start", "First string of list",                  &findarg_agent, &first_string}
};
   
int main(int argc, const char **argv)
{
   /* const char *prompts[] = { */
   /*    "_F_irst", */
   /*    "_P_revious", */
   /*    "_N_ext", */
   /*    "_L_ast", */
   /*    "_Q_uit" */
   /* }; */
   /* columnize_await_prompt(prompts, sizeof(prompts)/sizeof(prompts[0])); */
   /* return 0; */
   


   
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
         maxlen = get_max_string_len(first_string, end);

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
         columnize_string_array(first_string, end - first_string);
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

