#include <stdbool.h>
#include <ctype.h>    // for isdigit()

bool isJsonNumber(const char *str)
{
   bool retval = false;

   bool has_numerals = false;
   bool has_decimal = false;
   bool is_exponent = false;

   const char *ptr = str;
   int first_numeral = '\0';

   // Leading sign is permitted and skipped if found:
   if (*ptr == '-' || *ptr == '+')
      ++ptr;

   while (*ptr)
   {
      // RFC 8259 doesn't allow hex numbers, and we also
      // won't try to parse octal or binary prefixes
      if (isdigit(*ptr))
      {
         has_numerals = true;
         // Save first numeral for later check for forbidden initial '0'
         if (first_numeral ==  '\0')
            first_numeral = *ptr;
      }
      else if (*ptr == '.')
      {
         // .123 and -.123 are invalid
         if ( ptr==str || !isdigit(*(ptr-1)))
            goto early_exit;
         // encountering a second period or a period
         // in the exponent are invalid:
         else if (has_decimal || is_exponent)
            goto early_exit;
         else
            has_decimal = true;
      }
      else if (*ptr=='e' || *ptr=='E')
      {
         // [Ee] after exponent is an error:
         if (is_exponent)
            goto early_exit;

         // An exponent must have a coefficient (number before the 'e')
         if (!has_numerals)
            goto early_exit;

         // Next char can be a sign:
         if (*(ptr+1) == '-' || *(ptr+1) == '+')
            ++ptr;

         is_exponent = true;

         // Clear flag to require numerals after the E:
         has_numerals = false;
      }
      else
      {
         // Any unexpected characters rule-out proper number:
         goto early_exit;
      }

      ++ptr;
   }

   // It's only a number if several factors line up:
   retval = has_numerals && (first_numeral!='0' || is_exponent || has_decimal);

  early_exit:
   return retval;
}


/******
 * The following code and *stdio.h* include is only needed to
 * demonstrate the above isJsonNumber function.
 */

#include <stdio.h>

void run_test(const char *str)
{
   if (isJsonNumber(str))
      printf("\"\033[32;1m%s\033[39;22m\" IS a number!\n", str);
   else
      printf("\"\033[31;1m%s\033[39;22m\" IS NOT a number!\n", str);
}

const char *number_candidates[] = {
   "*Collection of valid JSON numbers",
   "1234",
   "12.34",
   "12e15",
   "1.2e5",
   "1.2e-5",
   "1.2e+5",
   "0e0",
   "0e+1",
   "0.123",

   "* ",
   "*Collection of invalid JSON numbers",
   "1.4e-5.3",   // decimal in exponent
   "0344",       // leading 0 is forbidden
   "1.2.4",      // multiple decimal points
   "1234jun",    // mixed number
   "1eE2",       // double 'e' notation
   "e15",        // expoennt without cofficient
   ".123",       // number without integer part

   "* ",
   "*Parsing JSON is a Minefield",
   "*Invalid Minefield numbers",
   "0x42",       // hex number not allowed (minefield)
   "0e+",        // exponent notation missing exponent
   ".2e-3",      // fraction part must follow a number, even if 0

   "* ",
   "*Valid Minefield numbers",
   "1e9999",                      // very large exponent
   "-237462374673276894279832",   // very large integer
   "123.456e-789"                 // complex exponent

};

int main(int argc, const char **argv)
{
   const char **ptr = number_candidates;
   const char **end = ptr + sizeof(number_candidates) / sizeof(number_candidates[0]);

   while (ptr < end)
   {
      if (**ptr == '*')
      {
         printf("\033[35;1m%s\033[39;22m\n", ((*ptr)+1));
      }
      else
         run_test(*ptr);

      ++ptr;
   }

   return 0;
}


/* Local Variables:                 */
/* compile-command: "gcc           \*/
/* -Wall -Werror -std=c99 -ggdb    \*/
/* -o isJsonNumber isJsonNumber.c"  */
/* End:                             */
