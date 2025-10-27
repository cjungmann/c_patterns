# C Patterns

A collection of diverse module source files with interesting solutions to
programming challenges.

<details>
   <summary>Table of Contents</summary>
   <ul>
      <li>
         <a href="#about_the_project">About the Project</a>
         <ul>
            <li><a href="#download_the_project">Download the Project</a></li>
            <li><a href="#using_the_project">Using the Project</a></li>
            <ul>
               <li><a href="#compiling">Compiling</a></li>
               <li><a href="#documentation">Documentation</a></li>
            </ul>
         </ul>
      </li>
      <li>
         <a href="#contents">Contents</a>
         <ul>
            <li><a href="#itoa">
               <b>itoa</b>:
               yet more implementations of integer to string
            </a></li>
            <li><a href="#perftest">
                <b>PerfTest</b>:
                records and reports on iterative timings
             </a></li>
            <li><a href="#read_line">
                <b>read_file_lines</b>:
                returns a file's contents by line
             </a></li>
            <li><a href="#commaize">
                <b>commaize</b>:
                format integer with commas
            </a></li>
            <li><a href="#columnize">
               <b>columnize</b>:
               arrange an array of string into columns
            </a></li>
            <li><a href="#arrayify">
                <b>arrayify</b>: break a string of words into an array
             </a></li>
            <li><a href="#get_keypress">
                <b>get_keypress</b>:
                for TUI programs to recognize keystrokes
            </a></li>
            <li><a href="initialize">
               <b>init_struct_array</b>:
               very old file, when I was younger and ignorant
            </a></li>
         </ul>
      </li>
   </ul>
</details>

## About the Project

This repository exists for me to gather non-trivial programming
solutions for future reference.

### Download the Project
~~~sh
git clone https://github.com/cjungmann/c_patterns.git
~~~

### Using the Project

The modules are generally independent.  Most include some code
that can be copied to another project, though some are incomplete
ideas that need work (*commaize.c*, I'm looking at you).

#### Compiling

Nearly all of the modules include a __Local Variables__ section
with a __compile-command__ that informs Emacs how to generate an
executable for demonstration and testing.

Modules that are meant to be directly included into another project
have the testing and *main* function code fenced behind an *\#ifdef*
block, which is triggered by the __compile-command__.  

#### Documentation

Documentation style is an ongoing project for me.  I pay attention
to what works and doesn't work and try to improve my meager skills
at documentation.  Newer modules will usually have better
documentation, though I may revisit old modules to improve things
if they're too bad.

I won't necessarily fix old modules.  Be prepared to dig around a
bit for useful stuff.

## Contents

Going forward, more recent efforts should be at the top in order
to prioritize the accumulation of experience in coding, documenting,
organizing, etc.

- <span id="itoa" />[Yet-another-itoa](README_itoa.md)  
  Called *itoa* to mirror library function *atoi*, this
  source file contains several variations of a function that
  converts an integer value to a string.  It includes some
  testing code that uses *snprintf* as a benchmark for speed
  and output.  It also uses [PerfTest][README_perftest.md) to
  compare the performance of the different versions.

- <span id="perftest" />[Performance Tester](README_perftest.md)  
  While considering the value of my new *itoa* function, it
  made sense to develop a suite of timing functions to compare
  the performance different *itoa* implementations.

- <span id="read_file" />[Line-reading Function](README_read_file_lines.md)  
  Collecting data from various sources usually requires processing
  files by line.  The main function, `read_file_lines` buffers
  the contents of a file and invokes a callback function to
  present a line to be processed.

- <span id="commaize" />commaize  
  This function, found in [commaize.c](commaize.c), uses recursion to
  break up a number from the small-end and displays it when
  the recursion unwinds.  I recently found out that you can do the same
  thing including an apostrophe in the format specifier when using
  *printf* when *setlocale* is used.

- <span id="columnize" />[Columns display][README_columnize.md)
  Displays a list of strings in column form, according to the
  screen dimensions.

- <span id="arrayify" />[Make an array of strings](README_arrayify.md)  
  This module can break a string into an array of strings,
  which is then made available through a callback function
  modeled after main() with a void return type.

- <span id="get_keypress" />get_keypress  
  Functions the provide immediate recognition of individual
  keypresses, including non-printable keys.  Source code in
  [get_keypress.c](get_keypress.c).

- <span id="initialize" />[Initializing Struct Arrays](README_init_struct_array.md)  
  I often iterate over static arrays to perform repetitve
  tasks like listing program options, simulate user responses,
  etc.  I frequently have trouble initializing these arrays.
  This guide should help me remember effective coding
  techniques.

