#ifndef COLUMNIZE_H
#define COLUMNIZE_H

/*
 * Typedef and two matching prototypes for two implementations
 * of columnar output.
 *
 * The functions expect an array of strings, with *end* being
 * a pointer to an imaginary element after the last element.
 * With that assumption, you can set the *end* value as
 * *start* + number_of_elements;
 *
 * The parameters *gutter*, *max_columns*, and *max_lines*
 * affect the output.  *gutter* is added to the length of the
 * longest list to make the column cell width.
 *
 * *max_columns* and *max_lines*, if not set to 0, 
 * 
 */

void get_screen_dimensions(int *wide, int *tall);
void tabulate_for_columns(const char **start, const char **end, int *maxlen);

const char ** display_newspaper_columns(const char **start,
                                        const char **end,
                                        int gutter,
                                        int max_columns,
                                        int max_lines);

const char ** display_parallel_columns(const char **start,
                                       const char **end,
                                       int gutter,
                                       int max_columns,
                                       int max_lines);

/*
 * Pager support follows, including typedefs and function prototypes.
 */

typedef enum columnize_pager_directions {
   CPR_NO_RESPONSE,
   CPR_QUIT,
   CPR_FIRST,
   CPR_PREVIOUS,
   CPR_NEXT,
   CPR_LAST
} CPRD;

typedef const char ** (*flow_function_f)(const char **start,
                                         const char **end,
                                         int gutter,
                                         int max_columns,
                                         int max_lines);

typedef CPRD (*page_control_f)(int page_current, int page_count);

typedef struct columnize_page_dims {
   flow_function_f flower;
   page_control_f  pcontrol;
   int             gutter;
   int             max_columns;
   int             reserve_lines;  // subtract from screen height to get max_lines for flow function
   int             paged_output;
} COLDIMS;

void columnize_default_dims(struct columnize_page_dims *dims);

CPRD columnize_default_controller(int page_current, int page_count);

void columnize_pager(const char **elements,
                     int element_count,
                     struct columnize_page_dims *dims);
   
#endif
