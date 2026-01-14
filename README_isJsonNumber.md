# Module isJsonNumber

This is (was) a quick-and-dirty effort to validate numbers
while parsing a JSON file without.  Using _strtod_ or _strtol_
are more permissive than JSON rules allow.

The source file includes a `main` function and some other code
that performs tests on a variety of numbers to confirm proper
evaluation.

## USAGE

```c
bool isJsonNumber(const char *str)
```

Simply pass a null-terminated string to `isJsonNumber`.  It
returns *true* if the string is a valid JSON number, *false*
if not.

## INCORPORATING TO YOUR PROJECT

Just copy function `isJsonNumber` into your project.

I'm not going to bother with my usual elaborate defines and
ifdefs.  The function is self-contained with the exception of
library function `isdigit` from **ctype.h** and the **bool**
datatype for the return value (found in **stdbool.h**).



