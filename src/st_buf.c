#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "st.h"
#include "st_buf.h"
#include "st_symb.h"
#include "st_color.h"

char *get_buf(int start_x, int start_y)
{
	int x_offset = start_x * pixel_byte_width;
	int y_offset = start_y * line_len;
	return (char *)(mp + x_offset + y_offset);
}

void fill_buf(int start_x, int start_y, int heigh, int width, unsigned short clr)
{
	int i, n;
	char *buf = get_buf(start_x, start_y);
	union color c = { .color = clr };

	for (n = 0; n < heigh; n++, buf += line_len) {
		for (i = 0; i < width; i++) {
			buf[i * 2 ] = c.rgb565.byte1;
			buf[i * 2 + 1] = c.rgb565.byte2;
		}
	}
}

void clear_buf(int start_x, int start_y, int heigh, int width)
{
	fill_buf(start_x, start_y, heigh, width, 0);
}

void draw_buf(char *symb, int start_x, int start_y, int heigh, int width, unsigned short clr)
{
	int i, n;
	char *buf = symb;
	union color c = { .color = clr };

	for (n = 0; n < heigh; n++) {
		for (i = 0; i < width; i++) {
			if (symb[i * 2] == 1) {
				buf[i * 2] = c.rgb565.byte1;
				buf[i * 2 + 1] = c.rgb565.byte2;
			}
		}
		symb += line_len; /* point to the next symbol string */
		buf = symb;
	}
}
