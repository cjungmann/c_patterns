# PerfTest Module

**PerfTest** is a performance testing module with a few goals:

- **Time-stamp Recorder**  
  nominally, to measure performance of various algorithms to
  learn which factors contribute or detract from overall
  performance.
- **C-based Object-oriented programming**  
  to play with designing an abstract interface, multiple
  implementations, and testing
- **Code Presentation**  
  to craft an easily scanned source file with comments that
  serve directly in the source and also compile with Doxygen
  for useful developer's documentation.
- **Built-in testing**  
  Compiling with Emacs includes code for `main()` and some
  testing functions that are excluded from compiling when the
  source file is included in another file.

I wrote the code for performance testing, but I spent most time
pursuing the other objectives mentioned above.

I want to have a style model for effective coding, and this
may not be it.  In the absence of other examples, I will begin
with this model in future efforts.

## The Interface


~~~c
struct PerfTest_s {
   void (*cleaner)(PerfTest *perfTest);
   bool (*add_point){PerfTest *perfTest);
   int (*points_count)(const PerfTest *perfTest);
   void (*get_points)(const PerfTest *peftTest, long *buffer, int bufferlen);
};
~~~

There are wrapper functions that cast instances of `PerfTest`-derived
objects to the generic form to access the interface methods.  For
example,

~~~c
void PerfTest_cleaner(PerfTest *pt)
~~~

will call the appropriate function pointer to run the cleaner function.

## NOTES

### Pause in Memory Allocation

_Switch to stack rather than heap memory allocation._

In testing that may not survive revisions, a bare loop ran only
to make time-stamps records.  At the 85th iteration, there is a
small but significantly longer pause between time-stamps, from
about 300 to about 2800 billionths of a second.  It is nearly
10 times as long, but would be vanishingly small in tests that
do real work.

The time-stamp values are saved in a singly linked list, and
the consistent location of the prolonged event suggested the
pause occurred during memory allocation.  I confirmed this by
increasing the size of the struct used for the linked list and
observing that the pauses happened consistently at more
frequent intervals.

**Proposal** I will modify the `add_point` function typedef
to include a `void*` argument that will be added to the linked
list struct to be used as seen fit.  Then, to test how stack
allocation affects the performance, the calling procedure will
allocate a leaf on the stack and pass the pointer as the
`void*` argument.

