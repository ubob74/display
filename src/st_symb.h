#ifndef _ST_SYMB_H_
#define _ST_SYMB_H_

struct symb {
	char **symb;
	int nr_symb;
	int heigh;
	int width;
	void (*load)(struct symb *, int);
};

void print_symbol(int i, int heigh, int width, int start_x, int start_y, unsigned short color);
void clear_symbol(int, int, int, int);
void init_symbols(void);
void populate_symbol(char *s, char *map, int heigh, int width);

#endif
