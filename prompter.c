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

char prompter_get_accented_letter_acc(const char *str, const Accenter *accenter)
{
   if (*str++ == *accenter->trigger && *str)
      return *str;
   else
      return '\0';
}

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

pr_bool prompter_extract_prompt_letter_acc(/*out*/ char *prompt_letter,
                                           const char *prompt,
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

char prompter_get_accented_letter(const char *str)
{
   return prompter_get_accented_letter_acc(str, global_accenter);
}

void prompter_print_prompt(const char *prompt, pr_bool accent)
{
   prompter_print_prompt_acc(prompt, accent, global_accenter);
}

void prompter_print_prompts(const char **prompts, int count_prompts)
{
   prompter_print_prompts_acc(prompts, count_prompts, global_accenter);
}

pr_bool prompter_extract_prompt_letter(/*out*/ char *prompt_letter, const char *prompt)
{
   return prompter_extract_prompt_letter_acc(prompt_letter, prompt, global_accenter);
}

void prompter_fill_letter_array(char **letters, int count, const char **prompts)
{
   prompter_fill_letter_array_acc(letters, count, prompts, global_accenter);
}

int prompter_await_prompt(const char **prompts, int count_prompts)
{
   return prompter_await_prompt_acc(prompts, count_prompts, global_accenter);
}

void prompter_pset_print(const PromptSet *set)
{
   prompter_print_prompts_acc(set->prompts, set->prompt_count, set->accenter);
}

int prompter_pset_await(const PromptSet *set)
{
   int result = prompter_await_prompt_acc(set->prompts, set->prompt_count, set->accenter);
   return set->results[result];
}

void prompter_punit_print_acc(const PUnit *units, int count, const Accenter *accenter)
{
   const PUnit *end = units + count;
   const PUnit *ptr = units;
   while (ptr < end)
   {
      if (ptr > units)
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

void prompter_punit_print(const PUnit *units, int count)
{
   prompter_punit_print_acc(units, count, global_accenter);
}

int prompter_punit_await_acc(const PUnit *units, int count, const Accenter *accenter)
{
   char *letters[count];

   int bufflen = prompter_initialize_letter_array(count, NULL, NULL, 0);
   char buffer[bufflen];
   prompter_initialize_letter_array(count, letters, buffer, bufflen);

   const PUnit *end = units + count;
   const PUnit *ptr = units;
   char **lptr = (char **)letters;
   while (ptr < end)
   {
      prompter_extract_prompt_letter_acc(*lptr, ptr->prompt, accenter);
      ++lptr;
      ++ptr;
   }

   int index = await_keypress((const char **)letters, count);
   assert(index >=0 && index < count);

   return units[index].value;
}

int prompter_punit_await(const PUnit *units, int count)
{
   return prompter_punit_await_acc(units, count, global_accenter);
}

void prompter_pmenu_print_acc(const PMenu *menu, const Accenter *accenter)
{
   prompter_punit_print_acc(menu->items, menu->count, accenter);
}

int prompter_pmenu_await_acc(const PMenu *menu, const Accenter *accenter)
{
   return prompter_punit_await_acc(menu->items, menu->count, accenter);
}

void prompter_pmenu_print(const PMenu *menu)
{
   prompter_punit_print_acc(menu->items, menu->count, global_accenter);
}

int prompter_pmenu_await(const PMenu *menu)
{
   return prompter_punit_await_acc(menu->items, menu->count, global_accenter);
}


/*
 * Erases console line and places cursor at left-most column.
 */
void prompter_reuse_line(void)
{
   // ESC[2K   erases current line
   // ESC[1G   moves cursor to column 1 of current line
   printf("\x1b[2K\x1b[1G");
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

void test_promptset(void)
{
   printf("This model uses PromptSet to simplify operation.\n");

   int transforms[] = {1, 2, 3, 4, 0};

   PromptSet ps = {
      prompts,
      transforms,
      ARRLEN(prompts),
      global_accenter };

   int result = -1;

   // quit returns 0 instead of 4 (position in string)
   while (result)
   {
      prompter_reuse_line();
      printf("Result was %3d.  ", result);
      prompter_pset_print(&ps);
      result = prompter_pset_await(&ps);
   }

   prompter_reuse_line();
}

void test_punit_gambit(void)
{
   PUnit punits[] = {
      { "&alphabetic", 12 },
      { "&frequency", 13 },
      { "&rank", 14 },
      { "&quit", 0 }
   };
   int punits_count = ARRLEN(punits);

   printf("This is a test of the PUnit feature.\n");


   int result = -1;
   while (result != 0) 
   {
      prompter_reuse_line();
      printf("Sorting order (%2d): ", result);
      prompter_punit_print(punits, punits_count);

      result = prompter_punit_await(punits, punits_count);
   }

   prompter_reuse_line();
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
   test_punit_gambit();
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

