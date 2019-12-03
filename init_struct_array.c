// -*- compile-command: "cc -Wall -Werror -ggdb -o init_struct_array init_struct_array.c" -*-

#include <stdio.h>
#include <unistd.h>

void wait_for_keypress()
{
   printf("\npress any ENTER to continue. Ctrl-C to exit.");
   getc(stdin);
   printf("\n");
}

const char *names_male[] = {
   "Tom",
   "Dick",
   "Harry"
};

/** For-loop iteration over simple array. */
void iterate_male_names()
{
   printf("[34;1mTest 1:[m iterate male names.\n");

   int len = sizeof(names_male) / sizeof(char*);
   for (int i=0; i<len; ++i)
      printf("%d: %s.\n", i, names_male[i]);

   wait_for_keypress();
}

const char *names_female[] = {
   "Jane",
   "Audrey",
   "Judy",
   NULL
};

/** Pointer-increment iteration over NULL-terminated array. */
void iterate_female_names()
{
   printf("\n[34;1mTest 2:[m iterate female names.\n");
   const char **ptr = names_female;
   while (*ptr)
   {
      printf("%s.\n", *ptr);
      ++ptr;
   }

   wait_for_keypress();
}

/**
 * Structure from which repeated calls to a function will return
 * a single name.  The **index** member tracks progress so the
 * function can return FALSE when there are no names left.
 *
 * This structure includes a *flexible array member* that can
 * be initialized when a variable is declared.
 */
typedef struct _comp_names
{
   unsigned int index;
   const char *names[];
} Comp_Names;

/** Example of initializing the flexible array member. */
Comp_Names cn_by_len= { 0, { "Tom", "Audrey", "Harry" } };

void iterate_comp_names_by_len()
{
   printf("\n[34;1mTest 3:[m Cannot iterate comp names by sizeof(struct object).\n");
   printf("The expected size of the struct would be, sizeof(int) + 3 * sizeof(char*) (%lu).\n",
          sizeof(unsigned int) + 3 * sizeof(char*));
   printf("sizeof() reported size: %lu.\n", sizeof(cn_by_len));
   printf("We're not executing this test, it causes a stack overflow.\n");

   /* // This would have been the test */
   /* int len = (sizeof(cn_by_len) - sizeof(unsigned int)) / sizeof(char*); */
   /* for (int i=0; i<len; ++i) */
   /*    printf("%d: %s.\n", i, cn_by_len.names[i]); */

   wait_for_keypress();
}

/** Example of initializing the flexible array member. */
Comp_Names cn_to_null= { 0, { "Tom", "Audrey", "Harry", NULL } };
void iterate_comp_names_to_null()
{
   printf("\n[34;1mTest 4:[m iterate comp names to null.\n");

   const char **ptr = cn_to_null.names;

   while (*ptr)
   {
      printf("%s\n", *ptr);
      ++ptr;
   }

   wait_for_keypress();
}



/**
 * A more complicated structure may include an array of
 * structs.  Consider the following two structures:
 */
typedef struct _person
{
   const char *fname;
   const char *lname;
} Person;

typedef struct _comp_object_array
{
   unsigned int index;
   Person person[];
} Comp_Obj_Array;


/** Let's attempt to create a variable: */

Comp_Obj_Array coa = { 0, {{"Betsy", "Ray" }, {"Tacy", "Kelly"}, {"Tib", "Miller"}} };

void iterate_comp_obj_by_len()
{
   printf("\n[34;1mTest 5:[m iterate comp obj by length.\n");
   printf("This will fail to work for the same reason as Test 3 above.\n");
   printf("The sizeof(struct) fails to account for the flexible array member,\n");
   printf("and would by inconclusive, anyway, because if undefined byte packing.\n");

   /* // This would have been the test: */
   /* int len = sizeof(coa) - sizeof(unsigned int); */
   /* len /= sizeof(Person); */
   
   /* for (int i=0; i<len; ++i) */
   /*    printf("%d: %s %s\n", i, coa.person[i].fname, coa.person[i].lname); */

   wait_for_keypress();
}

Comp_Obj_Array coan = {
   0,
   {
      {"Betsy", "Ray" },
      {"Tacy", "Kelly"},
      {"Tib", "Miller"},
      {NULL, NULL}
   }
};

void iterate_comp_obj_to_null()
{
   printf("\n[34;1mTest 6:[m iterate comp obj to NULLed struct pointer.\n");
   printf("While you can't have a NULL element, you can have an\n");
   printf("element with NULL members instead.\n");

   const Person *ptr = coan.person;
   while (ptr->fname)
   {
      printf("%s %s\n", ptr->fname, ptr->lname);
      ++ptr;
   }
}

int main(int argc, const char **argv)
{
   printf("There are 6 tests in this program.  Some are not actually\n");
   printf("executed because they would fail catastrophically.\n\n");

   iterate_male_names();
   iterate_female_names();
   iterate_comp_names_by_len();
   iterate_comp_names_to_null();
   iterate_comp_obj_by_len();
   iterate_comp_obj_to_null();

   return 0;
}
