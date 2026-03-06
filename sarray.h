/** @file */

#ifndef SARRAY_H
#define SARRAY_H

/**
 * @defgroup SARRAY SARRAY: a mini library for string arrays
 * @{
 */

/**
 * @brief Package of data that permits safe access to a static array of strings.
 *
 * Create this object with the @ref sarray_build function.  Access its
 * contents taking the address of the @p strings member and constrain
 * access by heeding the limit set by the @p count member.
 */
struct sarray_handle {
   unsigned   count;      ///< number of strings in array
   const char **strings;  ///< array of string pointers
};

/**
 * @brief Alias for struct sarray_handle
 */
typedef struct sarray_handle sarray_handle;

/**
 * @brief Called by the developer-provided @ref walker_func for
 *        each substring.
 *
 * The @p start and @p end parameters bear some explanation.
 * @p start is obvious, but @p end should point to the character
 * immediately following the final character that should be included
 * in the word.
 *
 * For example, given a string, "hi mom", @p start will point to
 * the 'h' in 'hi', and @p end will point to the space after the 'i'.
 *
 * The string at @p start will be copied, so it is not necessary to
 * preserve the source of the string between or after sarray calls.
 *
 * The function pointer and the data argument will differ according
 * to the needs of the phase in which is is called.
 * @param start    Points to the start of the substring
 * @param length   Length of string at @p start.
 * @param data     pass-through anonymous data pointer supplied to
 *                 the @ref walker_func by either @ref sarray_measure
 *                 or @ref sarray_build.
 */
typedef void(*put_string_func)(const char *start, int length, void *data);

/**
 * @brief Typedef for function that must be provided to @ref sarray_measure
 *        and @ref sarray_build
 *
 * The sarray tools must be provided a function with this signature
 * that they will use to get the collection of strings that will be
 * included in the resultant strings array.
 *
 * It is important that the function can repeat an identical list
 * over the two instances where it will be called.
 *
 * @param string   data to be processed, can be cast to another
 *                 datatype to match the datatype submitted to
 *                 the @p string argument of @ref sarray_measure
 *                 and @ref sarray_build.
 * @param func     function pointer through which substrings are
 *                 submitted for processing
 * @param data     data from the calling function to pass along to
 *                 the @p func callback function.
 */
typedef void(*walker_func)(const char *string, put_string_func func, void *data);

int sarray_measure(const char *string, walker_func func);

void sarray_build(sarray_handle *handle,
                  const char *string,
                  walker_func func,
                  char *buffer,
                  int bufflen);

const char *sarray_element_by_index(sarray_handle *handle, unsigned index);




/** @} End of SARRAY group */


// Content appendicies for skeleton headers above:

/**
 * @page SARRAY_C_SOURCE sarray.c source file
 * @verbinclude sarray.c
 */


/**
 * @addtogroup SARRAY
 *
 * @brief Tool for packing a set of strings into a memory block
 *
 * The product of this tool is a handle that holds a block of
 * memory that contains a packed collection of strings with an
 * array of pointers to the strings collection.  The handle also
 * contains the number of elements in the array to permit safe
 * access to the collection on strings.
 *
 * Learn how to use the tools with @ref SARRAY_DEMO
 */

/**
 * @page SARRAY_DEMO_PAGE Learn by Example
 *
 * @section SARRAY_DEMO Simple Implementation and Usage
 *
 * @subsection SARRAY_PARSING_FUNCTION Parsing Function Example
 *
 * To use this tool, a developer must provide a parsing function
 * that delivers a repeatable list of strings from a given source.
 * Among the arguments of the parsing function is a function
 * pointer that is to be called for each string that belongs in the
 * collection.
 *
 * The parsing function must deliver an identical set of strings
 * over two calls.  The first call is to determine the memory
 * requirements, the second call is to actually copy the strings
 * to the memory block.
 *
 * The demo section of @ref SARRAY_C_SOURCE includes parsing function
 * _custom_string_walker_.  It simple breaks a string into substrings
 * split by the spaces between words.
 *
 * @subsection SARRAY_BUILDING How to use *sarray* tools
 *
 * Besides creating a parser function, there are three steps to
 * creating an **sarray** handle:
 *
 * 1. Call _sarray_measure_ to get the memory requirements.
 * 2. Allocate memory of the required size.
 * 3. Call _sarray_build_ to pack the memory and initialize
 *    the sarray handle.
 *
 * A usage example can be found in @ref SARRAY_C_SOURCE in function
 * _run_demo_.
 *
 * @subsection SARRAY_USAGE Safely using the strings array
 *
 * The handle does not enforce safe usage, but only provides the
 * information needed to safely access the strings collection.
 * An example of how to safely use an *sarray* handle can be found
 * in @ref SARRAY_C_SOURCE in function _display_array_.
 *
 * @subsection SARRAY_BUILD_DEMO Instructions for building the demo
 *
 * Build the demo on the command line:
 *    `gcc -DSARRAY_MAIN -o sarray sarray.c`
 *
 * Build the demo in Emacs
 *    With Emacs open to _sarray.c_, call:
 *    `M-x compile`
 *
 *    There is a _Local Variables_ section at the bottom of
 *    the source file that defines the 
 */

#endif
