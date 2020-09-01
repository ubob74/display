#include "st_symb.h"
#include "st_symb_8x12.h"
#include "st.h"

#define MAP_WIDTH	7

static char smap0[][MAP_WIDTH] = {
	{2, 1, 3, 4},
	{2, 1, 2, 6},
	{4, 1, 1, 3, 6, 3},
	{4, 6, 1, 2, 7, 2},
	{4, 1, 1, 3, 6, 3},
	{2, 1, 2, 6},
	{2, 1, 3, 4},
};

static char smap1[][MAP_WIDTH] = {
	{2, 1, 5, 2},
	{2, 1, 4, 3},
	{2, 1, 3, 4},
	{2, 9, 5, 2}
};

static char smap2[][MAP_WIDTH] = {
	{2, 1, 2, 6},
	{2, 1, 1, 8},
	{4, 1, 1, 2, 7, 2},
	{2, 1, 7, 2},
	{2, 1, 6, 3},
	{2, 1, 5, 3},
	{2, 1, 4, 3},
	{2, 1, 3, 3},
	{2, 1, 2, 3},
	{2, 1, 1, 3},
	{2, 2, 1, 8}
};

static char smap3[][MAP_WIDTH] = {
	{2, 1, 2, 6},
	{2, 1, 1, 8},
	{4, 1, 1, 2, 7, 2},
	{2, 1, 7, 2},
	{2, 1, 7, 2},
	{2, 2, 4, 4},
	{2, 1, 7, 2},
	{2, 1, 7, 2},
	{4, 1, 1, 2, 7, 2},
	{2, 1, 1, 8},
	{2, 1, 2, 6},
};

static char smap4[][MAP_WIDTH] = {
	{4, 5, 1, 2, 7, 2},
	{2, 2, 1, 8},
	{2, 5, 7, 2},
};

static char smap5[][MAP_WIDTH] = {
	{2, 1, 1, 7},
	{2, 1, 1, 7},
	{2, 3, 1, 2},
	{2, 1, 1, 7},
	{2, 1, 1, 8},
	{2, 3, 7, 2},
	{2, 1, 1, 8},
	{2, 1, 1, 7},
};

static char smap6[][MAP_WIDTH] = {
	{2, 1, 2, 6},
	{2, 1, 1, 8},
	{4, 1, 1, 2, 8, 1},
	{2, 2, 1, 2},
	{2, 1, 1, 7},
	{2, 1, 1, 8},
	{4, 3, 1, 2, 7, 2},
	{2, 1, 1, 8},
	{2, 1, 2, 6},
};

static char smap7[][MAP_WIDTH] = {
	{2, 2, 1, 8},
	{4, 1, 1, 2, 7, 2},
	{2, 1, 7, 2},
	{2, 1, 6, 3},
	{2, 1, 6, 2},
	{2, 1, 5, 3},
	{2, 1, 5, 2},
	{2, 1, 4, 3},
	{2, 1, 4, 2},
	{2, 1, 3, 3},
	{2, 1, 3, 2},
};

static char smap8[][MAP_WIDTH] = {
	{2, 1, 2, 6},
	{2, 1, 1, 8},
	{4, 3, 1, 2, 7, 2},
	{2, 2, 2, 6},
	{4, 3, 1, 2, 7, 2},
	{2, 1, 1, 8},
	{2, 1, 2, 6},
};

static char smap9[][MAP_WIDTH] = {
	{2, 1, 2, 6},
	{2, 1, 1, 8},
	{4, 3, 1, 2, 7, 2},
	{2, 1, 1, 8},
	{2, 1, 2, 7},
	{2, 3, 7, 2},
	{2, 1, 2, 7},
	{2, 1, 2, 6},
};

static char *map_8x12[] = { (char *)&smap0, (char *)&smap1, (char *)&smap2, (char *)&smap3,
		(char *)&smap4, (char *)&smap5, (char *)&smap6, (char *)&smap7, (char *)&smap8, (char *)&smap9 };

static void load_symbol(struct symb *symb, int n)
{
	char *s = (char *)symb->symb[n];
	char *m = map_8x12[n];

	populate_symbol(s, m, symb->heigh, MAP_WIDTH);
}

struct symb symb_8x12 = {
	.nr_symb = ARRAY_SIZE(map_8x12),
	.heigh = 12,
	.width = 8,
	.load = load_symbol,
};
