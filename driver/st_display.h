#ifndef _ST_DISPLAY_H_
#define _ST_DISPLAY_H_

struct st_display {
	unsigned int width;
	unsigned int height;
	unsigned int regwidth;
	unsigned int buswidth;
	unsigned int bpp;
	unsigned int fps;
	const s16 *init_sequence;
};

extern struct st_display display;

struct device_driver;
void init_display(struct device_driver *driver);

#endif
