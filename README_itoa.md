# itoa.c, An Exploration of a Conversion

Converting an integer to a string is a frequent need, but the
logical function to do that, *itoa* is not standard.  So I rolled
my own *itoa*.  Three times.

## STUCK ON THE LANDING

This endeavor is part of a [larger repository][c_patterns] where I
save interesting solutions to programming challenges.

## OBJECTIVES

1. to develop a useful integer to string function
2. to explore documentation styles in search for a practical standard
3. to explore multiple conversion solutions
3. to measure and compare performance using [perftest][perftest]

### DEVELOP A USEFUL INTEGER TO STRING FUNCTION

The *itoa* function is not universally provided, and the fallback
integer-to-string function, *snprintf*, is very general and thus
likely to be relatively slow.  I thought it would be fun to make
yet-another *itoa* to practice some of the objectives listed above.

### EXPLORING DOCUMENTATION STYLES

I would like to have access to a model source file that demonstrates
not only good documentation style, but also a variety of Doxygen
constructs.  I would use this file as a reference, especially if I
step away from C programming for a while.

Not having found a definitive model online, This source file and
[perftest.c][perftest_source] have been developed to serve this
purpose until I find something better.

Included in these two source files are examples of pre- and
post-object comments, remote comments (eg function comment blocks
not attached to their functions).  I play with formatting that
improves readability, prioritizing readable source and ensuring that
the Doxygen output is effective.  For example, when names are
embedded in a Doxygen command like @b param or @b defgroup, I




sometimes isolate the name with a escaped newline immediately
following the name, or by enclosing the name in quotes.  I also
play with various grouping strategies.

### EXPLORE MULTIPLE SOLUTIONS

I wrote three version of *itoa*, each addressing possible drawbacks
of the other methods:

1. **int** itoa_recursive(**long** *value*, **int** *radix*, **char\*** buffer, **int** *bufflen*)  
   This is my first version, using recursion to reverse printing
   since we process conversions in the reverse order.

2. **int** itoa_loop(**long** *value*, **int** *radix*, **char\*** buffer, **int** *bufflen*)  
   Rewritten to run in a single stack frame, in case recursion has
   a measurable performance cost.

3. **const char*** itoa_instance(**long** *value*, **int** *radix*)  
   Rewritten to include a static buffer sufficiently large to contain
   the longest possible conversion.  The buffer is used both as the
   workspace and the return value is a pointer into the buffer to
   the beginning of the converted string.  It is inherently
   memory-safe at the sacrifce of thread-safety.

The first two versions, recursive and loop, theoretically require
two passes, first to predict the buffer length, then, after securing
an adequate block of memory, a second pass to do the conversion.
However, one could allocate and reuse a sufficiently large buffer
block in order to skip the first pass.

### MEASURE PERFORMANCE

The source includes tests for two types of performance, accuracy and
speed.  Look to the "COMPILING THE SOURCE FILE" section below to
prepare for testing.

The accuracy test compares the output of the various *itoa*
implementations against the output of *snprintf*, which is considered
to be the definitive output.  Of course, perfect accuracy is the only
acceptable outcome.

The speed test consists of a set of functions, each of which converts
an array of long values to strings, measuring the time of each
conversion.  The set of functions includes at least one test for each
of the *itoa* functions, and one for *snprintf*.  Each function
submits the timings to a reporting function that reports the fastest
and slowest conversions, as well as the mean, median, and standard
deviation of the series of timings.

## COMPILING THE SOURCE FILE

Compile the source file with the following command:
```sh
gcc -std=c99 -lm -o itoa itoa.c
```

If you are using Emacs, you can type `M-x compile` to compile the
file with extra warning messages and memory-checking in addition
to the above options.

## RUNNING THE TESTS

To run the test, simply type

~~~sh
./itoa
~~~

You can change the size of the array of long values from the detault
of 10,000 values by including a number argument:

~~~sh
./itoa 100000
~~~

Note that you might experience memory errors if you specify too large
a number.



[c_patterns]:      https://www.github.com/cjungmann/c_patterns.git
[perftest]:        README_perftest.md
[perftest_source]: perftest.c