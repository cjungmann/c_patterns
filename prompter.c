#include <stdio.h>
#include <memory.h>

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

char default_accenter_trigger = '_';
Accenter default_accenter = { prompter_accenter_color, &default_accenter_trigger };
Accenter *global_accenter = &default_accenter;

char prompter_get_accented_letter_acc(const char *str, const Accenter *accenter)
{
   if (*str == *accenter->trigger
       && *(++str)
       && *(str+1) == *accenter->trigger)
      return *str;
   else
      return '\0';
}

void prompter_print_prompt_acc(const char *prompt, int accent, const Accenter *accenter)
{
   char letter;

   if (!accenter)
      accenter = &default_accenter;

   while (*prompt)
   {
      if ((letter = prompter_get_accented_letter_acc(prompt, accenter)))
      {
         if (accent)
            accenter->accent_char(letter);
         else
            fputc(letter, stdout);

         prompt += 2;
      }
      else
         fputc(*prompt, stdout);

      ++prompt;
   }
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

int prompter_extract_prompt_letter_acc(const char *prompt,
                                       char *prompt_letter,
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
      prompter_extract_prompt_letter_acc(*prompts, *letters, accenter);
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

void prompter_print_prompt(const char *prompt, int accent)
{
   prompter_print_prompt_acc(prompt, accent, global_accenter);
}

void prompter_print_prompts(const char **prompts, int count_prompts)
{
   prompter_print_prompts_acc(prompts, count_prompts, global_accenter);
}

int prompter_extract_prompt_letter(const char *prompt, char *prompt_letter)
{
   return prompter_extract_prompt_letter_acc(prompt, prompt_letter, global_accenter);
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
   "_f_irst",
   "_p_revious",
   "_n_ext",
   "_l_ast",
   "_q_uit"
};
int count_prompts = sizeof(prompts) / sizeof(prompts[0]);

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
      sizeof(prompts)/sizeof(prompts[0]),
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

int main(int argc, const char **argv)
{
   /* test_fill_letter_array(); */
   /* test_await_prompt(); */
   test_promptset();
}

#endif



/* Local Variables: */
/* compile-command: "b=prompter; \*/
/*  cc -Wall -Werror -ggdb       \*/
/*  -std=c99 -pedantic           \*/
/*  -lreadargs                   \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"  \*/
/* End: */

