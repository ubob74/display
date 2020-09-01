#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "st.h"
#include "st_screen.h"
#include "st_symb.h"
#include "st_buf.h"
#include "st_color.h"

/* Gap in pixels between hours and minutes */
#define HM_GAP		12

#define CLOCK_START_X	5
#define CLOCK_START_Y	25

#define DOT_START_X		(CLOCK_START_X + 56)
#define DOT_Y1			(CLOCK_START_Y + 11)
#define DOT_GAP			15
#define DOT_Y2			(DOT_Y1 + DOT_GAP)

#define DATE_GAP		5
#define DATE_START_X	25
#define DATE_START_Y	(CLOCK_START_Y + 52)

#define TIMEOUT		10

/* Time context */
struct time_ctx {
	int num;
	int *cur;
	int *prev;
	int *x;
	int start_y;
	int heigh;
	int width;
};

static void init_time_ctx(struct time_ctx *tctx, int num)
{
	tctx->num = num;
	tctx->cur = calloc(num, sizeof(int));
	tctx->prev = calloc(num, sizeof(int));
	tctx->x = calloc(num, sizeof(int));

	assert(tctx->cur != NULL);
	assert(tctx->prev != NULL);
	assert(tctx->x != NULL);
}

static void release_time_ctx(struct time_ctx *tctx)
{
	free(tctx->cur);
	free(tctx->prev);
	free(tctx->x);
}

static void get_current_time(struct time_ctx *hm, struct time_ctx *date)
{
	time_t t;
	struct tm tm;

	t = time(NULL);
	localtime_r(&t, &tm);

	hm->cur[0] = tm.tm_hour / 10;
	hm->cur[1] = tm.tm_hour % 10;
	hm->cur[2] = tm.tm_min / 10;
	hm->cur[3] = tm.tm_min % 10;

	date->cur[0] = tm.tm_mday / 10;
	date->cur[1] = tm.tm_mday % 10;

	date->cur[2] = tm.tm_mon / 10;
	date->cur[3] = tm.tm_mon % 10 + 1;

	tm.tm_year += 1900;
	date->cur[4] = tm.tm_year / 1000;
	date->cur[5] = (tm.tm_year - date->cur[4] * 1000) / 100;
	date->cur[6] = (tm.tm_year - date->cur[4] * 1000 - date->cur[5] * 100) / 10;
	date->cur[7] = tm.tm_year % 10;
}

/* Initialize HOUR/MIN X axis */
static void init_hm(struct time_ctx *hm, int start_y, int heigh, int width)
{
	hm->x[0] = CLOCK_START_X;
	hm->x[1] = hm->x[0] + 24 + 3;
	hm->x[2] = hm->x[1] + 24 + HM_GAP;
	hm->x[3] = hm->x[2] + 24 + 3;

	hm->start_y = start_y;
	hm->heigh = heigh;
	hm->width = width;
}

static void show_digit(struct time_ctx *tctx, int n)
{
	tctx->prev[n] = tctx->cur[n];
	clear_symbol(tctx->x[n], tctx->start_y, tctx->heigh, tctx->width);
	print_symbol(tctx->cur[n], tctx->heigh, tctx->width,
			tctx->x[n], tctx->start_y, GREEN);
}

static void update(struct time_ctx *tctx)
{
	int i;

	for (i = 0; i < tctx->num; i++) {
		if (tctx->prev[i] != tctx->cur[i])
			show_digit(tctx, i);
	}
}

static void setup(struct time_ctx *tctx)
{
	int i;

	bzero(tctx->prev, tctx->num * sizeof(int));

	for (i = 0; i < tctx->num; i++)
		show_digit(tctx, i);
}

static void setup_dots(void)
{
	fill_buf(DOT_START_X, DOT_Y1, 3, 3, GREEN);
	fill_buf(DOT_START_X, DOT_Y2, 3, 3, GREEN);
}

static void update_dots(int v)
{
#if 0
	unsigned short color = (v != 0) ? GREEN : 0;
	fill_buf(DOT_START_X, DOT_Y1, 3, 3, color);
	fill_buf(DOT_START_X, DOT_Y2, 3, 3, color);
#endif
}

static void init_dots(void)
{
}

static void init_date(struct time_ctx *date, int start_y, int heigh, int width)
{
	/* Day */
	date->x[0] = DATE_START_X;
	date->x[1] = date->x[0] + 8 + 1;

	/* Month */
	date->x[2] = date->x[1] + 8 + DATE_GAP;
	date->x[3] = date->x[2] + 8 + 1;

	/* Year */
	date->x[4] = date->x[3] + 8 + DATE_GAP;
	date->x[5] = date->x[4] + 8 + 1;
	date->x[6] = date->x[5] + 8 + 1;
	date->x[7] = date->x[6] + 8 + 1;

	date->start_y = start_y;
	date->heigh = heigh;
	date->width = width;
}

#if 0
static void test_symb1(void)
{
	int i;
	init_symbols();

	while (1) {
		for (i = 0; i < 10; i++) {
			print_symbol(i, 40, 24, 50, 60, GREEN);
			update_screen();
			sleep(1);
			clear_symbol(50, 60, 40, 24);
			update_screen();
		}
	}
}
#endif

int main()
{
	int i;
	int ret;
	struct time_ctx hm;
	struct time_ctx date;
	struct update u;

	ret = open_screen();
	if (ret < 0)
		return -EINVAL;

	init_time_ctx(&hm, 4);
	init_time_ctx(&date, 8);

	init_symbols();
	init_hm(&hm, CLOCK_START_Y, 40, 24);
	init_date(&date, DATE_START_Y, 12, 8);
	init_dots();

	get_current_time(&hm, &date);

	setup(&hm);
	setup(&date);
	setup_dots();

	u.xs = 0;
	u.ys = 0;
	u.xe = 128;
	u.ye = 160;
	update_screen(&u);

	for (i = 0;;sleep(TIMEOUT)) {
		get_current_time(&hm, &date);
		update(&hm);
		update(&date);
		update_dots(i++ % 2);

		u.xs = 0;
		u.ys = 0;
		u.xe = 128;
		u.ye = 160;
		update_screen(&u);
	}

	release_time_ctx(&hm);
	release_time_ctx(&date);

	close_screen();
	return 0;
}
