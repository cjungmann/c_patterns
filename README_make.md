# Using Makefile Fragment make_c_patterns.mk

A software library is a collection of functions and data structures.
We are familiar with packaged libraries like *libc*, whose code
can be used by linking the library to the executable.  This page
describes my inspiration for including *c_patterns* functions
and data structures in a library-like manner.

This *c_patterns* project can be considered a library of sorts,
with modules being checked-out for other projects.  In fact, several
of the source code files were developed while working on my
[console-based theaurus project][1].  I broke down several reusable
tasks into separate source files that I added to this project and
then copied back into [th][1] through **make**.

If this makefile solution is interesting, you may also enjoy
looking at my [makefile_patterns project][2].

## How it Works

The makefile fragment, *make_c_patterns.mk*, includes rules
and variable assignments that

1. clones the *c_patterns* project in a subdirectory of the
   target project,
   
1. makes links of the source code of the requested modules
   into the SRC directory, then finally
   
1. initializes a variable that can be used to update the
   target prerequisites.  This may be necessary on the first
   build, where the prerequisite list may be compiled before
   the *c_patterns* source file links are available.

1. Other *make_c_patterns.mk* variables can be used for
   a clean target, as show below.

## Example Implementation

The following sparse makefile sample illustrates how
*make_c_patterns.mk* can be used to incorporate some *c_patterns*
sources into a project.

~~~make
TARGET = myproject
SRC = src

# Setting variables used by implicit rules:
PREFIX ?= /usr/local
CFLAGS = -Wall -Werror -std=c99 -pedantic -m64 -ggdb
LDFLAGS =

# The following makes a prerequisite list from C files in src
MODULES != ls -1 ${SRC}/*.c | sed 's/\.c/.o/g'

# The default rule (first rule in makefile is invoked with naked "make" call)
# must precede 'include make_c_patterns.mk' to prevent rules included
# therein from becoming the default rule.

# The first prerequiste of the default rule is a rule in make_c_patterns.mk.
all: CP_Prepare_Sources ${TARGET}

# Assign CP_NAMES to communicate desired modules, then add
# CP_OBJECTS to the project's prerequisite list.
CP_NAMES = get_keypress prompter columnise read_file_lines commaize
include make_c_patterns.mk
MODULES += ${CP_OBJECTS}

# Remove duplicates (operator $^ does this on GNU make, but not on BSD).
MODULES != echo ${MODULES} | xargs -n1 | sort -u | xargs

${TARGET} : ${MODULES}
          ${CC} -o $@ ${MODULES} ${LDFLAGS}
~~~

Use other *make_c_patterns.mk* variables for a clean target.
This example may go farther than you would, deleting source
code links from SRC and even the entire cloned *c_patterns*
directory.  The *c_patterns* project will be redownloaded and
linked at the next invocation of **make**.

~~~make
clean:
   rm -rf ${SRC}/*.o
   rm -rf ${SRC}/${CP_SOURCES}
   rm -rf ${SRC}/${CP_HEADERS}
   rm -rf c_patterns
~~~



[1]: https://github.com/cjungmann/th                "th thesaurus"
[2]: https://github.com/cjungmann/makefile_patterns "makefile_patterns"
