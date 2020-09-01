#ifndef _ST_SCREEN_H_
#define _ST_SCREEN_H_

struct update;

int open_screen(void);
void update_screen(struct update *);
void close_screen(void);

#endif
