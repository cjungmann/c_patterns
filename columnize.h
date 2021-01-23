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

typedef struct columnize_eliface {
   int (*get_len)(const void *el);
   int (*print)(FILE *f, const void *el);
   int (*print_cell)(FILE *f, const void *el, int width);
} CEIF;

int columnize_get_max_len(const CEIF *iface, const void **start, const void **end);

const void ** display_newspaper_columns(const CEIF *iface,
                                        const void **start,
                                        const void **end,
                                        int gutter,
                                        int max_columns,
                                        int max_lines);

const void ** display_parallel_columns(const CEIF *iface,
                                       const void **start,
                                       const void **end,
                                       int gutter,
                                       int max_columns,
                                       int max_lines);

extern CEIF ceif_string;

/*
 * Pager support follows, including typedefs and function prototypes.
 */

typedef enum columnize_pager_directions {
   CPR_NO_RESPONSE = -1,
   CPR_QUIT = 0,
   CPR_FIRST,
   CPR_PREVIOUS,
   CPR_NEXT,
   CPR_LAST
} CPRD;

typedef const void ** (*flow_function_f)(const CEIF *iface,
                                         const void **start,
                                         const void **end,
                                         int gutter,
                                         int max_columns,
                                         int max_lines);

struct columnize_page_dims;

typedef CPRD (*page_control_f)(int page_current, int page_count, struct columnize_page_dims *dims);

typedef struct columnize_page_dims {
   flow_function_f flower;
   page_control_f  pcontrol;
   int             gutter;
   int             max_columns;
   int             reserve_lines;  // subtract from screen height to get max_lines for flow function
   int             paged_output;
   void            *closure;
} COLDIMS;

void columnize_default_dims(struct columnize_page_dims *dims, void *closure);

CPRD columnize_default_controller(int page_current, int page_count, COLDIMS *dims);

void columnize_pager(const CEIF *iface,
                     const void **elements,
                     int element_count,
                     struct columnize_page_dims *dims);

void columnize_string_pager(const char **elements,
                            int element_count,
                            struct columnize_page_dims *dims);
   
#endif
