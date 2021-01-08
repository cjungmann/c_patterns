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
   
#endif
