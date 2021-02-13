#include <stdio.h>
#include <memory.h>
#include <assert.h>

#include "get_keypress.h"
#include "prompter.h"

void prompter_accenter_color(char letter)
{
   static const char hilite_on[] = "\x1b[32;1m";
   static const char hilite_off[] = "\x1b[m";
   fputs(hilite_on, stdout);
   fputc(letter, stdout);
   fputs(hilite_off, stdout);
}

char default_accenter_trigger = '&';
Accenter default_accenter = { prompter_accenter_color, &default_accenter_trigger, " / " };
Accenter *global_accenter = &default_accenter;

/*
 * Erases console line and places cursor at left-most column.
 */
void prompter_reuse_line(void)
{
   // ESC[2K   erases current line
   // ESC[1G   moves cursor to column 1 of current line
   printf("\x1b[2K\x1b[1G");
}

/*
 * Organized a blocks of memory into an array of pointers to char[2].
 *
 * This array is to be filled with letters for use by await_keypress()
 * to run the menu.
 *
 * Call with *count* and NULLs to calculate the buffer size neeed
 * for the operation.  Call a second time with an appropriately-sized
 * buffer and an array of pointers to pointers that will be initialized
 * with several pointers into the buffer.
 */
int prompter_initialize_letter_array(int count, char **letters, char *buffer, int bufflen)
{
   if (letters && buffer && bufflen == count*2)
   {
      memset(buffer, 0, bufflen);

      char **end = letters + count;
      while (letters < end)
      {
         *letters = buffer;
         buffer += 2;
         ++letters;
      }
   }

   return count * 2;
}

/*
 * Use accenter to identify the accented letter in *str
 */
char prompter_get_accented_letter_acc(const char *str, const Accenter *accenter)
{
   if (*str++ == *accenter->trigger && *str)
      return *str;
   else
      return '\0';
}

/* Call prompter_get_accented_letter_acc with global_accenter */
char prompter_get_accented_letter(const char *str)
{
   return prompter_get_accented_letter_acc(str, global_accenter);
}

/*
 * Fill a letters array, often prepared by prompter_initialize_letter_array(),
 * from a set of prompt strings.  The accenter argument will be used to
 * interpret the prompts to identify the letters.
 */
void prompter_fill_letter_array_acc(char **letters,
                                    int count,
                                    const char **prompts,
                                    const Accenter *accenter)
{
   if (!accenter)
      accenter = &default_accenter;

   const char **end = prompts + count;

   while (prompts < end)
   {
      prompter_extract_prompt_letter_acc(*letters, *prompts, accenter);
      ++letters;
      ++prompts;
   }
}

/*
 * Calls prompter_fill_letter_array() with global_accenter.
 */
void prompter_fill_letter_array(char **letters, int count, const char **prompts)
{
   prompter_fill_letter_array_acc(letters, count, prompts, global_accenter);
}

/*
 * Fill letters array from prompts in PMenu.
 */
void prompter_pmenu_fill_letter_array_acc(char **letters, const PMenu *menu, const Accenter *accenter)
{
   const PUnit *ptr = menu->items;
   const PUnit *end = ptr + menu->count;
   char **ppletter = (char **)letters;
   while (ptr < end)
   {
      prompter_extract_prompt_letter_acc(*ppletter, ptr->prompt, accenter);
      ++ppletter;
      ++ptr;
   }
}

/*
 * Print the prompt, using accenter to interpret it.  The *accent* flag
 * determines whether or not the accented character will be hilighted.
 */
void prompter_print_prompt_acc(const char *prompt, pr_bool accent, const Accenter *accenter)
{
   char letter;

   if (!accenter)
      accenter = &default_accenter;

   pr_bool accent_found = 0;

   while (*prompt)
   {
      if (!accent_found && (letter = prompter_get_accented_letter_acc(prompt, accenter)))
      {
         accent_found = 1;

         if (accent)
            accenter->accent_char(letter);
         else
            fputc(letter, stdout);

         ++prompt;
      }
      else
         fputc(*prompt, stdout);

      ++prompt;
   }

   // Signal if accent not found
   assert(accent_found);
}

void prompter_print_prompt(const char *prompt, pr_bool accent)
{
   prompter_print_prompt_acc(prompt, accent, global_accenter);
}


void prompter_print_prompts_acc(const char **prompts, int count_prompts, const Accenter *accenter)
{
   fputs("Press", stdout);

   const char **ptr, **end;
   for (ptr = prompts, end = prompts + count_prompts;
        ptr < end;
        ++ptr)
   {
      fputc(' ', stdout);
      prompter_print_prompt_acc(*ptr, 1, accenter);
   }

   // Flush characters that may not print without newline.
   fflush(stdout);
   
   // Move cursor to column 1
   fputs("\x1b[1G", stdout);
}

void prompter_print_prompts(const char **prompts, int count_prompts)
{
   prompter_print_prompts_acc(prompts, count_prompts, global_accenter);
}

pr_bool prompter_extract_prompt_letter_acc(/*out*/ char *prompt_letter, const char *prompt,
                                           const Accenter *accenter)
{
   if (!accenter)
      accenter = &default_accenter;

   while (*prompt)
   {
      if ((*prompt_letter = prompter_get_accented_letter_acc(prompt, accenter)))
         return 1;

      ++prompt;
   }

   return 0;
}

pr_bool prompter_extract_prompt_letter(/*out*/ char *prompt_letter, const char *prompt)
{
   return prompter_extract_prompt_letter_acc(prompt_letter, prompt, global_accenter);
}


int prompter_await_prompt_acc(const char **prompts, int count_prompts, const Accenter *accenter)
{
   if (!accenter)
      accenter = &default_accenter;

   int bufflen = prompter_initialize_letter_array(count_prompts, NULL, NULL, 0);
   char buffer[bufflen];
   char *letters[count_prompts];
   prompter_initialize_letter_array(count_prompts, letters, buffer, bufflen);

   prompter_fill_letter_array_acc((char**)letters, count_prompts, prompts, accenter);

   return await_keypress((const char **)letters, count_prompts);
}

int prompter_await_prompt(const char **prompts, int count_prompts)
{
   return prompter_await_prompt_acc(prompts, count_prompts, global_accenter);
}

void prompter_pmenu_print_acc(const PMenu *menu, const Accenter *accenter)
{
   const PUnit *ptr = menu->items;
   const PUnit *end = ptr + menu->count;

   while (ptr < end)
   {
      if (ptr > menu->items)
      {
         if (accenter->divider)
            fputs(accenter->divider, stdout);
         else
            fputc(' ', stdout);
      }

      prompter_print_prompt_acc(ptr->prompt, 1, accenter);
      ++ptr;
   }
}

void prompter_pmenu_print(const PMenu *menu)
{
   prompter_pmenu_print_acc(menu, global_accenter);
}

int prompter_pmenu_await_acc(const PMenu *menu, const Accenter *accenter)
{
   char *letters[menu->count];

   int bufflen = prompter_initialize_letter_array(menu->count, NULL, NULL, 0);
   char buffer[bufflen];
   prompter_initialize_letter_array(menu->count, letters, buffer, bufflen);
   prompter_pmenu_fill_letter_array_acc(letters, menu, accenter);

   int index = await_keypress((const char **)letters, menu->count);
   assert(index >=0 && index < menu->count);

   return menu->items[index].value;
}

int prompter_pmenu_await(const PMenu *menu)
{
   return prompter_pmenu_await_acc(menu, global_accenter);
}

#ifdef PROMPTER_MAIN

#include "get_keypress.c"

const char *prompts[] = {
   "&first",
   "&previous",
   "&next",
   "&last",
   "&quit"
};
int count_prompts = ARRLEN(prompts);

void test_fill_letter_array(void)
{
   int bufflen = prompter_initialize_letter_array(count_prompts, NULL, NULL, 0);
   char buffer[bufflen];
   char *letters[count_prompts];
   prompter_initialize_letter_array(count_prompts, letters, buffer, bufflen);

   prompter_fill_letter_array((char**)letters, count_prompts, prompts);

   for (int i=0; i<count_prompts; ++i)
   {
      printf("%s\n", letters[i]);
   }
}

void test_await_prompt(void)
{
   printf("This is a DIY (mostly) model.\n");
   int index = 0;

   while (index != 4)
   {
      prompter_print_prompts(prompts, count_prompts);
      index = prompter_await_prompt(prompts, count_prompts);

      printf("\nYou pressed ");
      prompter_print_prompt(prompts[index], 0);
      printf("\n");
   }
}

void test_pmenu_gambit(void)
{
   PUnit punits[] = {
      { "&alphabetic", 12 },
      { "&frequency", 13 },
      { "&rank", 14 },
      { "&quit", 0 }
   };
   PMenu pmenu = { punits, ARRLEN(punits) };

   printf("This is a test of the PMenu feature.\n");


   int result = -1;
   while (result != 0) 
   {
      prompter_reuse_line();
      printf("Sorting order (%2d): ", result);
      prompter_pmenu_print(&pmenu);

      result = prompter_pmenu_await(&pmenu);
   }

   prompter_reuse_line();
}

int main(int argc, const char **argv)
{
   /* test_fill_letter_array(); */
   /* test_await_prompt(); */
   /* test_promptset(); */
   test_pmenu_gambit();
}

#endif



/* Local Variables: */
/* compile-command: "b=prompter; \*/
/*  cc -Wall -Werror -ggdb       \*/
/*  -std=c99 -pedantic           \*/
/*  -lreadargs                   \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"  \*/
/* End: */

