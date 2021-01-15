# Module arrayify

Converts a delimited string into a series of individual strings,
an array to which is prepared to be handed off to a callback function.

The original purpose of this module was to make it easier to test
the parsing of command line parameters, but it may also serve other
uses.  I feel like I have had other occasions to do this.

## Main Features

- Create an array of char* from a delimited string of char.
- Uses shell precedent to parse the string:
  - IFS (Internal Field Separator) value is used to identify
    element breaks.  An explicitly set value is the first
    priority, then if not set, the second chance is the environment's
    IFS value , then if not set, falls back to conventional
    space-tab-newline combination.
  - Escape notation works like shell, for example, `\n` is
    treated like a newline may be treated as an IFS char, which
    a backslash at the end of a line escapes the newline so that
    particular newline is not treated as an IFS char.
  - **NOTE** No special treatment of quote or apostrophe characters to
    enclose a element.  Escape IFS characters with a backslash
    to prevent breaking.  (This may change if deemed useful.)
  - **NOTE** no parameter expansion is performed.  Use the real
    shell to create arrays if that is necessary.
- Provides a closure parameter to make variables in the scope of
  the calling function available to the callback function.

## Demo Code Using *arrayify_file*

This most common usage is to test several different collections of
command line arguments by reading the collections from files.  The
following code sample should demonstrate how easy it is to do.

~~~c
#include <stdio.h>
#include "arrayify.h"

void dummy_main(int argc, const char **argv, void *closure)
{
   int count = 0;
   const char **end = argv + argc;
   while (argv < end)
   {
      printf("%2d: %s\n", ++count, *argv)
      ++argv;
   }
}

int main(int argc, const char **argv)
{
   if (argc < 2)
   {
      printf("Please enter a filename for parsing.\n");
      return 1;
   }
   else
   {
      // Optional: set the IFS to \n only so as to break only on newline:
      arrayify_set_ifs("\n");

      arrayify_file(argv[1], dummy_main, NULL);
      return 0;
   }
}
~~~

## Alternate Usage without Callback

The simple-use functions, `arrayify_string` and `arrayify_file`
allocate memory and invoke a callback with the results.  If callbacks
conflict with your coding practices, this service this module provides
can be accessed directly, as the following example demonstrates.

~~~c
#include <stdio.h>
#include <string.h>
#include "arrayify.h"

const char number_list[] = "one two three four five six";

int main(int argc, const char **argv)
{
      // Optional: set the IFS to \n only so as to break only on newline:
      arrayify_set_ifs("\n");

      int slen = strlen(number_list);
      int count = arrayify_parser(number_list, slen, NULL, 0);
      if (count>0)
      {
         const char* strarray[count];
         arrayify_parser(number_list, slen, strarray, count);

         const char **ptr = strarray;
         const char **end = strarray + count;

         for (; ptr < end; ++ptr)
            printf("%s\n", *ptr);
      }
      return 0;
   }
}
~~~


## Preparing to use arrayify.c

Copy arrayify.c and arrayify.h into your project's directory.



