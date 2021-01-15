#ifndef ARRAYIFY_H
#define ARRAYIFY_H


typedef void (*arrayify_user_f)(int argc, const char **argv, void *closure);

// arrayify_parser uses an IFS string to split a string.  The
// default is, conventionally, space, tab, newline.   An IFS
// value set with this function will override the default value
// and any IFS value set in the environment.
void arrayify_set_ifs(const char *newifs);

// Double-purpose function, to count elements when *els* is NULL,
// then to fill the *els* array with separated strings if *els* is included.
int arrayify_parser(char *buffer, int bufflen, const char **els, int elslen);

// Short cut, using arrayify_parser, to deliver an array of strings.
void arrayify_string(char *buffer, int bufflen, arrayify_user_f user, void *closure);

// Short cut, using arrayify_string, to allocate buffer and fill with contents of a file.
int arrayify_file(const char *filepath, arrayify_user_f user,  void *closure);

#endif
