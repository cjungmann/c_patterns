#include <stdio.h>

/**
 * Add commas to printout of integer values.
 *
 * Using recursion to read from the end to the beginning
 * of the number to properly break into 1000s, then print
 * each 3-digit portion when the recursion unwinds.
 *
 * The same thing can be done with *setlocale* and *printf*
 * using a format specifier with an apostrophe:
 *
 * char *oldloc = setlocale(LC_NUMERIC,NULL);
 * setlocale(LC_NUMERIC, "");
 * printf("Watch this: %'d.\n", 1000000);
 * setlocale(LC_NUMERIC,oldloc);
 */
void commaize_number(unsigned long num)
{
   if (num > 0)
   {
      // recurse to reverse order
      commaize_number(num / 1000);

      if (num >= 1000)
         printf(",%03lu", num % 1000);
      else
         printf("%lu", num);
   }
}


#ifdef COMMAIZE_MAIN

void test(unsigned long num)
{
   printf("%7lu: ", num);
   commaize_number(num);
   printf("\n");
}

int main(int argc, const char **argv)
{
   test(100);
   test(1000);
   test(10000);
   test(100000);
   
   return 0;
}


#endif



/* Local Variables: */
/* compile-command: "b=commaize; \*/
/*  cc -Wall -Werror -ggdb        \*/
/*  -std=c99 -pedantic            \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"   \*/
/* End: */
