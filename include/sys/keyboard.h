#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#define KB_BUF_SIZE 4096
extern char kb_line[KB_BUF_SIZE];
extern int line_ready;
char read_key();
void insert_key(char ch);

#endif
