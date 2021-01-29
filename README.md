# C/C++ Patterns

I'm tired of repeatedly searching for obscure patterns to
remind myself how to accomplish some programming goal.  This
project will be a dumping ground for code that works for
a variety of situations.

Each pattern will include a README file and a code file
that can be compiled to perform some tests.

Rather than including a Makefile, each source file will
include a compile string in an EMACS *file local variables*
line.  In EMACS, compile with `M-x compile`.

- [Line-reading Function](README_read_file_lines.md)
  Collecting data from various sources usually requires processing
  files by line.  The main function, `read_file_lines` buffers
  the contents of a file and invokes a callback function to
  present a line to be processed.

- commaize  
  This function, found in *commaize.c*, uses recursion to
  break up a number from the small-end and displays it when
  the recursion unwinds.

- [Make an array of strings](README_arrayify.md)  
  This module can break a string into an array of strings,
  which is then made available through a callback function
  modeled after main() with a void return type.

- get_keypress  
  Functions the provide immediate recognition of individual
  keypresses, including non-printable keys.
  
- [Initializing Struct Arrays](README_init_struct_array.md)  
  I often iterate over static arrays to perform repetitve
  tasks like listing program options, simulate user responses,
  etc.  I frequently have trouble initializing these arrays.
  This guide should help me remember effective coding
  techniques.

  
  
  


