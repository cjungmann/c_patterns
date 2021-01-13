#ifndef ARRAYIFY_H
#define ARRAYIFY_H

typedef int (*arrayify_user)(int argc, const char **argv);
void arrayify_string(char *buffer, int bufflen, arrayify_user user);

#endif
