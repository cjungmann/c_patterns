# Function *read_file_lines*

Many times have I written new code to read a file line-by-line.
In the interest of reusing code, I created a shared-object
library, but that seemed like overkill for a simple function.
In the future, I hope to remember to use this function from
this project instead.

## Contents

- **read_file_lines.c**  
  This file contains the function and a global buffer-length
  variable.  It also includes #ifdef-protected code that can
  be compiled for testing.

- **read_file_lines.h**  
  This header file two main items of interest, a prototype
  for function *read_file_lines*, and a typedef for a callback
  function that *read_file_lines* calls for each line that
  it reads.

  I made a typedef for **bool** to communicate the intended
  return value: 0 for failure, non-0 for success.

- **read_file_lines.test**  
  This is a simple text file that was used to test and debug
  the *read_file_lines* function, as well as function *commaize*,
  which is another pattern in this project.

## Using *read_file_lines*

Copy *read_file_lines.c* and *read_file_lines.h* to your project
and include them in your build.  They should work without much
trouble.

### Compiling Test Code

I have successfully compiled the code in both GNU-Linux and BSD
environments.

Unaltered, the test code requires my
[readargs library](https://www.github.com/cjungmann/readargs.git), but
a competent developer should be able to modify the code to remove
this dependency.

If you are using EMACS, and assuming you have also installed **readargs**,
compiling the test code is a simple as typing `M-x compile`.  If you're
not using EMACS, or you want to compile from the command line, use:

~~~sh
cc -lreadargs -DREAD_FILE_LINES_MAIN -o read_file_lines read_file_lines.c
~~~

Of course, if you have purged the **readargs** code, you may remove the
*-lreadargs* parameter.
