#ifndef _ST_COLOR_H_
#define _ST_COLOR_H_

#define BLACK	0x0000
#define BLUE	0x1000
#define GREEN	0x0004
#define RED		0x0080

struct rgb565 {
	unsigned char byte1;
	unsigned char byte2;
};

union color {
	unsigned short color;
	struct rgb565 rgb565;
};

#endif
