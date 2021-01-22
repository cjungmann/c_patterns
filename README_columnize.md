# Source Listing **columnize.c**

My objective for this topic changed shortly after beginning.
Go down to **Conversion Specifiers in `printf`** to see what
I learned about *printf* conversion specifiers.

## Patterns Included in the Source

There are three goals attempted in the *columnize.c* source file
- Functions that output data in columns
- [readargs](https://www.github.com/cjungmann/readargs.git) usage with two examples of custom agents.
- String formatting with *printf* family of functions

## Columnar Output Formatting

Let's first talk about the two flow options for columnar data.
These are *newspaper* and *parallel* flow.

- "Newspaper flow" with consecutive strings are below each other in
  each column, like text copy in a newspaper.  This is the default
  flow used by **ls**.  Look at the following example of *newspaper*
  formatting:

~~~sh
  1  5  9 
  2  6 
  3  7 
  4  8
~~~

  Although it's easy to scan consecutive items when they are lined
  up, it's not convenient when the output extends beyond the bottom
  of the view.  In that case, the reader will have to move the view
  up and down to follow the flow of the content.

- "Parallel flow" where consecutive strings are added to the right.
  The **ls** command can output this flow by using the **-x** option.
  This flow pattern may make it easier to read ordered lists if the
  output overflows the view.  Look at the following example of
  *parallel* formatting:

~~~sh
  1  2  3  4  5
  6  7  8  9
~~~

   This flow is easier to produce, and avoids the view port problems
   for output that extends beyond the bottom of the view, but it's
   much harder to scan consecutive items.

## Using *columnize.c*

There are two functions, `display_newspaper_columns` and `display_parallel_columns`,
that have the same signature (argument types and return value) so they
can be toggled through a function pointer.

Both functions expect a pointer to an array of char* pointers and a pointer
to a hypothetical element just past the array that is used as an interation
boundary.  Following these two arguments and display control arguments,
*gutter*, *max_columns*, and *max_lines*, which can constrain the output
if used.

The function returns the array element immediately following the last
printed element.  This makes it easy to generate paged output by simply
looping:

~~~c
   const char *strings[] = { "one", "two", "three", "four", "five", "six", "sevel" };
   
   const char **top = strings;
   const char **end = strings + (sizeof(strings) / sizeof(strings[0]));
   while (top < end)
   {
      // Print list in sets of two-column by two-row matricies, with
      // a minimum 2-character space between elements
      top = display_newspaper_columns(top, end, 2, 2, 2);
   }
~~~

Note how using *end* as an iteration boundary rather than an element count,
the same value can be used for each call to `display_newspaper_columns`,
rather than having to recalculate the remaining element count as the
*top* pointer changes.

This document does not attempt to show all uses of *columnize.c*  Please
inspect the source code for additional usage ideas.

## Example of *readargs* usage

I use my [readargs library](https://www.github.com/cjungmann/readargs.git)
in the testing section of the source code.  In this particular listing,
there are examples of using custom *readargs* agents.

- **flow_agent** is a custom agent that sets a function pointer based on
  the flow preference of the user.

- **findarg_agent** is an unusual agent that is not called by the user,
  but is invoked by *readargs* to identify the first non-option command
  line argument.  The test code needs to know when the options end and
  the data arguments begin, and this agent helps do that.


## Conversion Specifiers in `printf`

This was the initial inspiration for creating *columnize.c*, but is
only an after thought now.

The conversion specifiers in `printf` can include numbers that control
padding and output length.  I was familiar using these numbers when
printing integers and floating type values.  It turns out that these
numbers also affect how a string is printed in ways unfamiliar to me.

Another new realization was that the width and precision specifications
can be replaced with asterisks to be filled with integer values in
the arguments following the format string.

The string control and variable specifiers are particularly useful.
This section discusses some code patterns for exploiting these
features.

### Printing an Unterminated String

When working with raw data records or when printing a substring
from a larger string, you may not be able to depend on an `'\0'`
string terminator.  In these cases, you may need to tell `printf`
how many characters to print.

~~~c
// The argument, str, is not guaranteed to be terminated with an \0,
// but string_length does accurately indicates the string length.
void show_print(const char *str, int string_length)
{
   printf("%.*s", string_length, str);
}
~~~

### Printing Out a List of Spaces

I used to print a variable number of spaces with a *for-loop*
or a specially constructed string.  There's a much easier way.

~~~c
int spaces_to_print = 10;
printf("%*s", spaces_to_print, "");
~~~

The conversion specifier consumes two arguments, the asterisk consumes an
integer and the `s` consumes a _const char*_.  To print only spaces, apply
an empty string to the `s` token.

I wanted column strings to be left-justified.  By default, `printf` outputs
right-justified text.  Not realizing that numeric formatting specifiers
worked on strings, my columnar output printed the word, then printed a
number of spaces to advance the cursor to the beginning of the next string.

~~~c
// Needlessly complicated justification method.
// print spaces after string to position cursor.
//
// This function assumes that cell_width is guaranteed
// to be greater than the number of characters in *str.
void just_print(const char *str, int cell_width)
{
   int len = strlen(str);
   printf(str);
   printf("%*s", cell_width - len, "");
}
~~~

It turns out that the `-` (the dash (AKA minus) character) will left-
justify a string just like it does with a number.  Thus, the following
statement produces the same output as the above function.

~~~c
printf("%-*s", cell_width, str);
~~~

### My Unexpected Mistake

I didn't understand how the asterisks and period worked in the
conversion specifier, so I was surprised with the output of the
following code:

~~~c
int cell_width = 5;

const char *str = "bogus";
printf("%*.s", cell_width, str);     // expected "bogus", printed 5 spaces
printf("%.*", cell_width, "");       // expected 5 spaces, got NO spaces
~~~

### Combining Padding and Unterminated String

Using both the width and precision values of the conversion specifier,
you can pad and limit printing in one specifier:

~~~c
void padded_print(const char *str, int string_length, int output_width)
{
   // right-justified (padding left of string)
   printf("%*.*s", output_width, string_length, str);
   // left-justified (padding right of string)
   printf("%-*.*s", output_width, string_length, str);
}
~~~



### Demonstration of *printf* outputs

Compile the source file *columnize.c* and run the program like this:

~~~sh
./columnize -d
~~~


