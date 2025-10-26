# C/C++ Patterns

This project is a collection of small scripts containing solutions
that I might want to reuse or review in the future.

## HOW IT'S MEANT TO WORK

This repository mostly consists of independent source files, many
of which also have a README file that discusses the programming
challenge addressed by the source file.

Most of the source files include code that demonstrates and tests
the solution, along with an Emacs-specific setting that can be called
to compile the file which can be used to run the tests.  They can
be compiled in Emacs with `M-x compile`.

For those not using Emacs, the compilation flags used can be found
in the value of the *compile-comment* variable in the *Variables*
section at the bottom of the source file.

## CONTENTS

Rather than including a Makefile, each source file will
include a compile string in an EMACS *file local variables*
line.  In EMACS, compile with `M-x compile`.

Look at [this convenient tool](README_make.md) that provides a
convenient means for incorporating these modules into other
projects.

- [Line-reading Function](README_read_file_lines.md)  
  Collecting data from various sources usually requires processing
  files by line.  The main function, `read_file_lines` buffers
  the contents of a file and invokes a callback function to
  present a line to be processed.

- commaize  
  This function, found in [commaize.c](commaize.c), uses recursion to
  break up a number from the small-end and displays it when
  the recursion unwinds.

- [Columns display][README_columnize.md)
  Displays a list of strings in column form, according to the
  screen dimensions.

- [Make an array of strings](README_arrayify.md)  
  This module can break a string into an array of strings,
  which is then made available through a callback function
  modeled after main() with a void return type.

- get_keypress  
  Functions the provide immediate recognition of individual
  keypresses, including non-printable keys.  Source code in
  [get_keypress.c](get_keypress.c).

- [Initializing Struct Arrays](README_init_struct_array.md)  
  I often iterate over static arrays to perform repetitve
  tasks like listing program options, simulate user responses,
  etc.  I frequently have trouble initializing these arrays.
  This guide should help me remember effective coding
  techniques.

- [Yet-another-itoa](README_itoa.md)  
  Called *itoa* to mirror library function *atoi*, this
  source file contains several variations of a function that
  converts an integer value to a string.  It includes some
  testing code that uses *snprintf* as a benchmark for speed
  and output.  It also uses [PerfTest][README_perftest.md) to
  compare the performance of the different versions.

- [Performance Tester](README_perftest.md)
  

