# Source Listing **columnize.c**

My objective for this topic changed shortly after beginning.
Go down to **Conversion Specifiers in `printf`** to see what
I learned about *printf* conversion specifiers.

## Patterns Included in the Source

There are three concepts contained in the source file
- A columnar output utility
- [readargs](https://www.github.com/cjungmann/readargs.git) usage with two examples of custom agents.
- String formatting with *printf* family of functions



## Columnar Output Formatting

### Two Flow Options for Columnal Output

There are two types of column flow, *newspaper* and *parallel*.

- "Newspaper flow" with consecutive strings are below each other in
  each column, like text copy in a newspaper.  Look at the following
  example of *newspaper* formatting:

~~~sh
  1  4  7
  2  5  8
  3  6  9
~~~

  Although it's easy to scan consecutive items when they are lined
  up, it's not convenient when the output extends beyond the bottom
  of the view.  In that case, the reader will have to move the view
  up and down to follow the flow of the content.

- "Parallel flow" where consecutive strings are added to the right.
  Look at the following example of *parallel* formatting.

~~~sh
  1  2  3
  4  5  6
  7  8  9
~~~

   This flow is easier to produce, and avoids the view port problems
   for output that extends beyond the bottom of the view, but it's
   much harder to scan consecutive items.














- A custom *readargs* agent. Refer to `findarg_reader()`, `findarg_writer()`,
  the agent definition following the two functions, and the *list_start raAction
  in the action map.


## Conversion Specifiers in `printf`

At the risk of disappointing the reader, I'm only showing a couple things
related to adding spaces.

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
printf("%*.s", cell_width, str);     // this prints 5 spaces instead of "bogus"
printf("%.*", cell_width, "");       // expected 5 spaces, got no spaces
~~~

### Demonstration of *printf* outputs

Compile the source file *columnize.md* and run the program like this:

~~~sh
./columnize -d
~~~


