#ifndef COLUMNIZE_H
#define COLUMNIZE_H

void get_screen_dimensions(int *wide, int *tall);

/*
 * This interface abstracts the information needed to
 * prepare a column view of data.  The ceif_string is the
 * simplest implementation of this for string elements, but
 * an ambitious developer might create an interface for
 * formatting a list that includes index numbers or another
 * characteristic.
 */
typedef struct columnize_el_iface {
   int (*get_len)(const void *el);
   int (*print)(FILE *f, const void *el);
   int (*print_cell)(FILE *f, const void *el, int width);
} CEIF;

// Simple interface for strings.
extern CEIF ceif_string;

int columnize_get_max_len(const CEIF *iface, const void **start, const void **end);
int get_max_string_len(const char **start, const char **end);


/* Typedef so we can toggle between two different column flow models. */
typedef const void ** (*flow_function_f)(const CEIF *iface,
                                         const void **start,
                                         const void **end,
                                         int gutter,
                                         int max_columns,
                                         int max_lines);


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

/*
 * Pager support follows, including typedefs and function prototypes.
 */
typedef enum columnize_pager_directions {
   CPR_NO_RESPONSE = -1,
   CPR_QUIT = 0,
   CPR_FIRST,
   CPR_PREVIOUS,
   CPR_NEXT,
   CPR_LAST,
   CPR_CUSTOM
} CPRD;


typedef struct pager_params {
   const void **start;
   const void **end;

   int gutter;
   int reserve_lines;
   int max_size;

   const void **ptr;

   int win_wide;
   int win_tall;
   int columns_to_show;
   int lines_to_show;
   int page_capacity;
} PPARAMS;

void PPARAMS_init(PPARAMS *params,
                  const void **start,
                  int el_count,
                  int gutter,
                  int reserve_lines,
                  int max_size);

void PPARAMS_query_screen(PPARAMS *params);

typedef const void** (*PPARAMS_mover)(PPARAMS *params);
extern PPARAMS_mover pparams_movers[];

const void** PPARAMS_first(PPARAMS *params);
const void** PPARAMS_previous(PPARAMS *params);
const void** PPARAMS_next(PPARAMS *params);
const void** PPARAMS_last(PPARAMS *params);
int PPARAMS_page_count(const PPARAMS *params);
int PPARAMS_pointer_index(const PPARAMS *params, const void **ptr);
const void** PPARAMS_move(PPARAMS *params, CPRD request);

void columnize_print_progress(PPARAMS *params, const void **stop);




   
#endif
