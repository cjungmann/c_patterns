<a id="readme_top" />

# itoa

Implementations of integer-to-string for fun and education

<details>
  <summary>Table of Contents</summary>
  <ul>
    <li><a href="#about-the-project">About the Project</a></li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#download">Download</a></li>
        <li><a href="#integer-size-setting">Integer Size Setting</a></li>
        <li><a href="#compile">Compile</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a>
      <ul>
        <li><a href="#as-copy-source">As Copy Source</a></li>
        <li><a href="#in-compiled-form">In Compiled Form</a></li>
      </ul>
    </li>
    <li><a href="#implementations">Implementations</a>
      <ul>
        <li><a href="#processing-order">Processing Order</a></li>
        <li><a href="#itoa-functions">Itoa Functions</a>
          <ul>
            <li><a href="#itoa_recursive">itoa_recursive</a></li>
            <li><a href="#itoa_loop">itoa_loop</a></li>
            <li><a href="#itoa_instant">itoa_instant</a></li>
          </ul>
        </li>
      </ul>
      <li><a href="#testing">Testing</a></li>
    </li>
  </ul>
</details>

## About the Project

Since the useful function **itoa** is sometimes but not always
available, I wanted to make my own version to try out some ideas.

There is an acceptable way to do this with the standard library,
using one of the `printf` variations.  I wanted to do my own
anyway.

See the [Implementations](#implementations) section below for
information about what's available and the difference between
the implementations.

The testing code utilizes perfomance testing functions found in
the [PerfTest](README_perftest.md) module.

It may also be interesting to peruse the module's [source code](itoa.c)
to learn about the various implementations.

<p style="align-right"><a href="#readme_top">Back to top</a></p>

## Getting Started

There are multiple implementations of *itoa*.  You can compile the
module and run the tests, or copy one of the implementations to your
own project.

<p style="align-right"><a href="#readme_top">Back to top</a></p>

### Download

~~~sh
git clone https://www.github.com/cjungmann/c_patterns.git
cd c_patterns
~~~

<p style="align-right"><a href="#readme_top">Back to top</a></p>

### Integer Size Setting

While there is a data type named `integer`, this module can specify
different integer sizes from `char` to `long long`.  There is a
section called **Integer-type Settings** near the top of the file
where the integer type can be set.  Follow the instructions in the
source file to change it.

<p style="align-right"><a href="#readme_top">Back to top</a></p>

### Compile

Keeping in mind that this module is a wrapper around a set of
functions that perform the integer-to-string conversion, it can be
compiled to run some tests.

There are two ways to compile the module:

1. While the source file is open in **Emacs**, perform a *compile*
   with `M-x compile`.  The **Local Variables** section defines a
   compile command with the appropriate options, including some
   warning options enabled.

2. From the command line, type:
   ```sh
   cc -std=c99 -lm -o itoa itoa.c
   ```

<p style="align-right"><a href="#readme_top">Back to top</a></p>

## Usage

The intended application of the module is to be a source from which
one would copy one of multiple [implementations](#implementations)
of **itoa** that it contains.  Look at
[As Copy Source](#as-copy-source) below.

That said, the module also contains code for comparing the
performance of the various integer conversion methods.  The section
[In Compiled Form](#in-compiled-form) how to compile the module and
what you can expect to see.

### As Copy Source

Some care must be taken to copy an implementation to another
project:

1. At least one of the integer type settings must accompany any of
   conversion implementations.
2. Ensure the target source file includes *assert.h*, *string.h*,
   and *limits.h*.
3. To use **itoa_recursive**, you must also copy
   **itoa_recursive_copy** to the target project.

### In Compiled Form

The compiled **itoa** program does the following:

- Generates an array of random integer values that will be shared
  by all test executions.
- Each test will convert every number in the array to a string,
  recording the time of completion in order to compile a record
  of times needed to perform the conversions.
- For each test, when the conversions are complete, will print a
  small statistical analysis report including fastest and slowest
  times, mean, median, and standard deviation of the times.

The default number of integers in the array is 10,000.  A different
array size can be specified by passing an integer value to the
**itoa** command.  For example, entering:

```sh
./itoa 100000
```
will create an integer array of 100,000 values instead of the default
10,000 values.

<p style="align-right"><a href="#readme_top">Back to top</a></p>

## Implementations

### Processing Order

Remember that when doing addition or subtraction on paper, you
work from the right side of the number in case you have to carry.
Likewise, converting digits of of an integer must work from the
least significant side of the value.  However, the numerals in the
string are printed from most to least significant digits.

<p style="align-right"><a href="#readme_top">Back to top</a></p>

### Itoa Functions

There are three new implementations with different strategies.
The first two are modeled after `snprintf` in that they will
report the buffer requirements of a number if no buffer is
provided.  Buffers can then be tailored to the requirements of
the number being converted.

1. **itoa_recursive**  
   Uses recursion to reverse the digit order.  The drawback is
   that it requires a companion function, **itoa_recursive_copy**.

   This was my first take on the project.  I think that using
   recursion is an elegant way to handle copying the most
   significant digits in the opposite order of their being
   determined.

2. **itoa_loop**  
   Copies the output from the end of the output string, using a
   decremented iterator to move leftwards as the digits are
   discerned.

   I developed this version to compare its performance against the
   recursive method.  I anticipated that the recursive method would
   be slower and wanted to know the performance penalty.

3. **itoa_instant**  
   Performance testing of the first two versions revealed that
   memory allocation has a time cost, and I wanted an **itoa**
   version that completely avoids memory allocation.  This version
   uses a static buffer that is large enough for the worst case
   conversion, converting the maximum value into binary.

   This version is not an exact match for the other two
   implementations because the returned value is only valid
   until the next call to **itoa_instant**.  One of the test
   implementations addresses this unfair advantage by copying
   the result into another string before recording the time
   interval.

<p style="align-right"><a href="#readme_top">Back to top</a></p>

## Testing

In addition to comparing between the different implementations,
a true test must consider the performance of the built in
standard function, `snprintf`.

Additionally, there are two tests for **itoa_instant**, one just
to do the conversion without copy because that's how it most
effectively might be used, and the second a handicap verion that
copies the output to another string like the other three methods
to consider another interpretation of "fair."

Note that these tests do not test conversion to alternate number
bases because **snprintf** is not capable of that feature.

1. **convert_with_snprintf**  
   The standard method, measure the performance to determine if
   there is much value in using any of these implementations.
2. **convert_with_itoa_recursive**  
   Using my preferred method.
3. **convert_with_itoa_loop**
   Originally predicted to be a better performer than
   **itoa_recursive**, but the results contradicted that prediction.
4. **convert_with_itoa_instant**  
   Run the conversions with the best case scenario for
   **itoa_instant**, that is without copying the string.  This
   was the fasted method.
4. **convert_with_itoa_instant_copy**  
   This is the handicapped version that runs **itoa_instant**,
   uses `strlen` to identify the buffer requirements, then allocates
   the memory and copies the string.  Even with this handicap, it
   is clearly more efficient than the other versions

<p style="align-right"><a href="#readme_top">Back to top</a></p>

[gcc]:    https://gcc.gnu.org/
