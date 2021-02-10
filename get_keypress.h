#ifndef GET_KEYPRESS_H
#define GET_KEYPRESS_H

void gk_hide_cursor(void);
void gk_show_cursor(void);

int get_keypress(char *buff, int bufflen);
int await_keypress(const char **keys, int keycount);



#endif
