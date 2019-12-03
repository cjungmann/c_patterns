# Struct Array Initialization

I frequently create arrays of data over which I plan to iterate
for various tasks.  I would like the initialization code to be as
simple and obvious to read as possible.

I also would like the iteration code to be elegant, efficient, and
simple to understand.

## Terms For Searching



- **non-defining declaration**  
  An array defined without a size `char name[] = "Tom";`

- **flexible array member**
  A non-defining array declaration is allowed if it's the last
  member of a struct.

- [Associated File](init_struct_array.c)

- **Associated File:** init_struct_array.c

