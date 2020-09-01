#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "st_symb.h"
#include "st_buf.h"
#include "st.h"

#include "st_symb_8x12.h"
#include "st_symb_24x40.h"

static struct symb *symbols[] = {
	&symb_8x12,
	&symb_24x40,
};

static struct symb *find_symbol(int heigh, int width)
{
	int i;
	struct symb *s;

	for (i = 0; i < ARRAY_SIZE(symbols); i++) {
		s = symbols[i];
		if (s->heigh == heigh && s->width == width)
			return s;
	}
	return NULL;
}

void print_symbol(int i, int heigh, int width, int start_x, int start_y, unsigned short color)
{
	struct symb *s = find_symbol(heigh, width);
	if (!s)
		return;

	if (i >= s->nr_symb)
		return;

	s->symb[i] = get_buf(start_x, start_y);
	s->load(s, i);

	draw_buf(s->symb[i], start_x, start_y, s->heigh, s->width, color);
}

void clear_symbol(int start_x, int start_y, int heigh, int width)
{
	clear_buf(start_x, start_y, heigh, width);
}

static inline void allocate_symbols(struct symb *symb)
{
	symb->symb = malloc(symb->nr_symb);
	assert(symb->symb);
}

void populate_symbol(char *s, char *map, int heigh, int map_width)
{
	int i, n, nr, nr_line;

	for (n = 0; n < heigh; map += map_width) {
		nr = map[0] + 1;
		nr_line = map[1];
		n += nr_line;
		while (nr_line--) {
			for (i = 2; i < nr; i += 2) {
				int n1 = map[i] - 1;
				int n2 = map[i + 1] + n1;
				while (n1 < n2)
					*(s + 2 * n1++) = 1;
			}
			s += line_len;
		}
	}
}

void init_symbols(void)
{
	int i;
	struct symb *symb;

	for (i = 0; i < ARRAY_SIZE(symbols); i++) {
		symb = symbols[i];
		allocate_symbols(symb);
	}
}
