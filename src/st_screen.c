#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <assert.h>

#include "st.h"
#include "st_screen.h"

#define SCREEN_NAME		"/dev/st"
#define STGET_SCREENINFO 	_IOW('s', 0, struct screeninfo)
#define ST_UPDATE 			_IO('s', 1)

unsigned char *mp;
int line_len;
int pixel_byte_width;

/**
 * Screen info
 */
struct screeninfo {
	unsigned int xres;
	unsigned int yres;
	unsigned int bpp;
	unsigned int line_length;
};

/**
 * stfd - screen descriptor
 * mp - mmaped pointer
 * ssize - HW screen size
 */
struct screen {
	int stfd;
	unsigned char *mp;
	unsigned long ssize;
	struct screeninfo si;
};

static struct screen s;

int open_screen(void)
{
	int ret;
	struct screeninfo *si = &s.si;

	s.stfd = open(SCREEN_NAME, O_RDWR);
	if (s.stfd == -1)
		return -1;

    /* Get fixed screen information */
	ret = ioctl(s.stfd, STGET_SCREENINFO, si);
	if (ret < 0)
		goto err;

#ifdef DEBUG
	printf("ret=%d %dx%d, %dbpp\n", ret, si->xres, si->yres,
			si->bpp);
#endif

	s.ssize = si->xres * si->yres * si->bpp / 8;
	assert(s.ssize != 0);

	/* Map the device to memory */
	s.mp = (unsigned char *)mmap(0, s.ssize, PROT_READ | PROT_WRITE,
			MAP_SHARED, s.stfd, 0);
	if (!s.mp) {
		perror("Error: failed to map framebuffer device to memory");
		goto err;;
	}

	fprintf(stderr, "map=%lx\n", (unsigned long)s.mp);

	memset((void *)s.mp, 0, s.ssize);

	mp = s.mp;
	line_len = si->line_length; /* line length */
	pixel_byte_width = si->bpp / 8; /* bits per pixel */

	return 0;
err:
	close(s.stfd);
	return -1;
}

void update_screen(struct update *update)
{
	ioctl(s.stfd, ST_UPDATE, update);
}

void close_screen(void)
{
	munmap(s.mp, s.ssize);
	close(s.stfd);
}
