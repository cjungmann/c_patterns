#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>   // for exit()
#include <string.h>
#include <stdio.h>    // for dprintf() upon buffer overflow error

#include "get_keypress.h"

struct termios _termios_start;

/*
 * We use a function pointer so the first use can do some
 * setup and then reset the function pointer for subsequent
 * calls to set raw-mode.
 */
typedef void (*raw_mode_t)(void);

/* prototypes for raw-mode setting candidate functions */
void gk_set_raw_mode_first(void);
void gk_set_raw_mode_impl(void);

raw_mode_t gk_set_raw_mode = gk_set_raw_mode_first;

/*
 * The first call to set raw-mode tries to start termios.
 * Upon success, the function pointer will be set to the
 * function that assumes the termios is already started.
 */
void gk_set_raw_mode_first(void)
{
   int result = tcgetattr(STDIN_FILENO, &_termios_start);
   if (result)
   {
      const char *msg = "\ntcgetattr failed: aborting.\n";
      write(STDERR_FILENO, msg, strlen(msg));
      exit(1);
   }
   else
   {
      gk_set_raw_mode = gk_set_raw_mode_impl;
      (*gk_set_raw_mode)();
   }
}

/*
 * Set raw-mode function that assumes termios is started.
 */
void gk_set_raw_mode_impl(void)
{
   struct termios traw = _termios_start;

   // Unset some input mode flags
   traw.c_iflag &= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );

   // Unset some output mode flags
   traw.c_oflag &= ~( OPOST );

   // Unset some control mode flags
   traw.c_cflag &= ~( CS8 );

   // Unset some local mode flags
   /* traw.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG ); */
   traw.c_lflag &= ~( ECHO | ICANON | IEXTEN );

   tcsetattr(STDIN_FILENO, TCSAFLUSH, &traw);
}

void gk_unset_raw_mode(void)
{
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &_termios_start);
}

void gk_set_read_mode(unsigned min_chars, unsigned timeout)
{
   struct termios tcur;
   tcgetattr(STDIN_FILENO, &tcur);
   tcur.c_cc[VMIN] = min_chars;
   tcur.c_cc[VTIME] = timeout;
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &tcur);
}

void gk_set_default_read_mode(void)
{
   struct termios tcur;
   tcgetattr(STDIN_FILENO, &tcur);
   tcur.c_cc[VMIN] = _termios_start.c_cc[VMIN];
   tcur.c_cc[VTIME] = _termios_start.c_cc[VTIME];
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &tcur);
}

/*
 * Function returns 0 until the user presses a key.
 * Loop to wait for a keypress.
 */
int get_keypress(char *buff, int bufflen)
{
   int bytes_read;
   char *ptr = buff;

   // Set keypress detection conditions:
   (*gk_set_raw_mode)();
   gk_set_read_mode(1, 1); // Wait for at least 1 char, no longer than 1/10th second

   bytes_read = read(STDIN_FILENO, ptr, bufflen);

   if (bytes_read && bytes_read < bufflen)
   {
      ptr += bytes_read;
      *ptr = '\0';
   }
   else
      ptr = buff; // signal error

   // Restore normal conditions
   gk_set_default_read_mode();
   gk_unset_raw_mode();

   if (ptr == buff)
      fprintf(stderr, "\n\x1b[31;1mctt_get_keypress buffer overflow.\x1b[m\n");

   return ptr > buff;
}

int await_keypress(const char **keys, int keycount)
{
   int maxlen = 0;

   if (keys)
   {
      const char **end = keys + keycount;
      for (const char **ptr=keys; ptr < end; ++ptr)
      {
         int slen = strlen(*ptr);
         if (slen > maxlen)
            maxlen = slen;
      }
      
      char buff[maxlen + 1];
      while(get_keypress(buff, sizeof(buff)))
      {
         for (const char **ptr=keys; ptr < end; ++ptr)
         {
            if (!strcmp(*ptr, buff))
               return ptr - keys;
         }
      }
   }
   else
   {
      // Wait for any key if no keys passed
      char buff[10];
      while (!get_keypress(buff, sizeof(buff)))
         ;
   }

   return -1;
}

#ifdef GET_KEYPRESS_MAIN

#include <stdio.h>
const char test_prefix[] = 
   "Press a key to see its output, 'q' to quit.\n"
   "Try function keys, use the control key, etc.\n"
   "\n"
   "Control characters will be red with a ^ prefix,\n"
   "\e[31m^[\e[m is the escape key press.\n"
   "\n";

void print_char_vals(const char *str)
{
   for (const char *ptr=str; *ptr; ++ptr)
   {
      if (iscntrl(*ptr))
         printf("[31m^%c[m", (*ptr)+64);
      else
         printf("%c", *ptr);
   }

   printf(" (");

   for (const char *ptr=str; *ptr; ++ptr) 
      printf("\\x%02x ", *ptr);

   printf(") ");
  
}

void test_buff_size(int buffsize)
{
   char buff[buffsize];
   int count = 0;

   printf(test_prefix);

   while ( get_keypress(buff, buffsize) && *buff != 'q' )
   {
      printf("%-3d: ", ++count);
      print_char_vals(buff);
      printf("\n");
   }
}

void test_await(void)
{
   printf("Test new function, await_keypress().\n"
          "This function waits for one of a list of\n"
          "keypresses before exiting, returning the\n"
          "index of the keypress selected.\n");

   const char* keys[] = {
      "q",
      "n",
      "\x0b"    // ENTER
   };

   int val = await_keypress(keys, sizeof(keys)/sizeof(keys[0]));
   if (val < 0)
      printf("keypress aborted.\n");
   else
      printf("You pressed %s (item %d).\n", keys[val], val);
}

int main(int argc, const char **argv)
{
   printf("A keypress may return multiple characters.\n"
          "Press an arrow key and three characters are returned.\n"
          "F-keys return up to seven characters.\n"
          "\n"
          "As a result, the size of the character buffer does.\n"
          "matter.  The following is a test of two sizes of buffer.\n"
          "\n");
   printf("Test with 10 character buffer.\n");
   test_buff_size(10);

   printf("\n\nTest with a 3 character buffer.\n"
          "Many keystrokes will overflow the buffer.\n"
          "This will likely terminate prematurely.\n");
   test_buff_size(3);

   test_await();
}

#endif

/* Local Variables: */
/* compile-command: "b=get_keypress; \*/
/*  gcc -Wall -Werror -ggdb \*/
/*  -D${b^^}_MAIN           \*/
/*  -o $b ${b}.c"           \*/
/* End: */

