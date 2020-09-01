#ifndef _ST_H_
#define _ST_H_

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

struct update {
	int xs;
	int ys;
	int xe;
	int ye;
};

extern int line_len;
extern int pixel_byte_width;
extern unsigned char *mp;

#endif
