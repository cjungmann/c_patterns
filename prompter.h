#ifndef PROMPTER_H
#define PROMPTER_H

typedef void (*accenter_f)(char letter);

/* Bundle customizable necessaities. */
typedef struct accenter_t {
   accenter_f accent_char;
   char       *trigger;       // points to default_accenter_trigger
} Accenter;

/*
 * The *trigger member of *global_accenter points to
 * this variable.  Change the value to change the character
 * that identifies the character accent.
 */
extern char default_accenter_trigger;   // defaults to '_', which can be changed

extern Accenter default_accenter; 

/*
 * This pointer is used by the functions that don't take
 * an Accenter parameter.
 *
 * Points to Accenter instance default_accenter at first,
 * but can be replaced with a custom Accenter, in which case,
 * the non-accenter functions will use the custom Accenter.
 */
extern Accenter *global_accenter;


char prompter_get_accented_letter_acc(const char *str,
                                      const Accenter *accenter);

void prompter_print_prompt_acc(const char *prompt, int accent,
                               const Accenter *accenter);

void prompter_print_prompts_acc(const char **prompts, int count_prompts,
                                const Accenter *accenter);

int prompter_extract_prompt_letter_acc(const char *prompt, char *prompt_letter,
                                       const Accenter *accenter);

int prompter_await_prompt_acc(const char **prompts, int count_prompts,
                              const Accenter *accenter);

int prompter_initialize_letter_array(int count, char **letters, char *buffer, int bufflen);

void prompter_fill_letter_array_acc(char **letters, int count, const char **prompts,
                                    const Accenter *accenter);

int prompter_await_prompt_acc(const char **prompts, int count_prompts, const Accenter *accenter);


char prompter_get_accented_letter(const char *str);
void prompter_print_prompt(const char *prompt, int accent);
void prompter_print_prompts(const char **prompts, int count_prompts);
int prompter_extract_prompt_letter(const char *prompt, char *prompt_letter);
void prompter_fill_letter_array(char **letters, int count, const char **prompts);
int prompter_await_prompt(const char **prompts, int count_prompts);


#endif