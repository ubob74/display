#ifndef _ST_BUF_H_
#define _ST_BUF_H_

char *get_buf(int start_x, int start_y);
void draw_buf(char *, int, int, int, int, unsigned short);
void fill_buf(int, int, int, int, unsigned short);
void clear_buf(int, int, int, int);

#endif
