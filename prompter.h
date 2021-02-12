#ifndef PROMPTER_H
#define PROMPTER_H

typedef int pr_bool;

#ifndef ARRLEN
#define ARRLEN(X) (sizeof(X) / sizeof(X[0]))
#endif

/*
 * The Accenter contains information needed to
 * accent a specific letter in a prompt string
 * to indicate the character that triggers the prompt.
 */
typedef void (*accenter_f)(char letter);

/* Bundle customizable necessaities. */
typedef struct accenter_t {
   accenter_f accent_char;
   char       *trigger;       // points to default_accenter_trigger
   const char *divider;
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

typedef struct prompter_unit {
   const char *prompt;
   int value;
} PUnit;

typedef struct prompter_menu {
   const PUnit *items;
   int count;
} PMenu;

/*
 * A PromptSet holds several bits of data
 * to simplify the display and interpretation
 * of a set of prompt strings.
 */
typedef struct prompter_set {
   const char **prompts;
   int *results;
   int prompt_count;
   Accenter *accenter;
} PromptSet;


char prompter_get_accented_letter_acc(const char *str,
                                      const Accenter *accenter);

void prompter_print_prompt_acc(const char *prompt, pr_bool accent,
                               const Accenter *accenter);

void prompter_print_prompts_acc(const char **prompts, int count_prompts,
                                const Accenter *accenter);

pr_bool prompter_extract_prompt_letter_acc(/*out*/ char *prompt_letter,
                                           const char *prompt,
                                           const Accenter *accenter);

int prompter_await_prompt_acc(const char **prompts, int count_prompts,
                              const Accenter *accenter);

int prompter_initialize_letter_array(int count, char **letters, char *buffer, int bufflen);

void prompter_fill_letter_array_acc(char **letters, int count, const char **prompts,
                                    const Accenter *accenter);

int prompter_await_prompt_acc(const char **prompts, int count_prompts, const Accenter *accenter);


char prompter_get_accented_letter(const char *str);
void prompter_print_prompt(const char *prompt, pr_bool accent);
void prompter_print_prompts(const char **prompts, int count_prompts);
pr_bool prompter_extract_prompt_letter(/*out*/ char *prompt_letter, const char *prompt);
void prompter_fill_letter_array(char **letters, int count, const char **prompts);
int prompter_await_prompt(const char **prompts, int count_prompts);

void prompter_pset_print(const PromptSet *set);
int prompter_pset_await(const PromptSet *set);

void prompter_punit_print_acc(const PUnit *units, int count, const Accenter *accenter);
int prompter_punit_await_acc(const PUnit *units, int count, const Accenter *accenter);
void prompter_punit_print(const PUnit *units, int count);
int prompter_punit_await(const PUnit *units, int count);

void prompter_pmenu_print_acc(const PMenu *menu, const Accenter *accenter);
int prompter_pmenu_await_acc(const PMenu *menu, const Accenter *accenter);
void prompter_pmenu_print(const PMenu *menu);
int prompter_pmenu_await(const PMenu *menu);

void prompter_reuse_line(void);



#endif
