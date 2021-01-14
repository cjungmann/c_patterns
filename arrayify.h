#ifndef ARRAYIFY_H
#define ARRAYIFY_H


typedef void (*arrayify_user_f)(int argc, const char **argv, void *closure);

void arrayify_string(char *buffer, int bufflen, arrayify_user_f user, void *closure);
int arrayify_file(const char *filepath, arrayify_user_f user,  void *closure);

#endif
