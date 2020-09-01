#ifndef _ST_H_
#define _ST_H_

#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
#include <linux/types.h>

struct device;
struct st_param;

struct st_deferred_io {
	unsigned long delay;
	struct mutex lock;
	void (*deferred_io)(struct st_param *);
};

struct vm_pages {
	unsigned long *vm_pages;
	int nr_pages;
};

struct st_bitfield {
	__u32 offset;
	__u32 length;
};

struct st_param {
	char id[16];
	struct miscdevice mdev;
	struct spi_device *spi;
	struct device *device;
	struct {
		int reset;
		int dc;
	} gpio;
	const s16 *init_sequence;
	struct vm_pages vm_pages;
	__u32 xres;
	__u32 yres;
	__u32 line_length;
	__u32 bits_per_pixel;
	struct st_bitfield red;
	struct st_bitfield blue;
	struct st_bitfield green;
	struct delayed_work deferred_work;
	struct st_deferred_io *stdefio;
};

struct screeninfo {
	__u32 xres;
	__u32 yres;
	__u32 bpp;
	__u32 line_length;
};

struct update {
	int xs;
	int ys;
	int xe;
	int ye;
};

#define STGET_SCREENINFO 	_IOW('s', 0, struct screeninfo)
#define ST_UPDATE 			_IO('s', 1)

#endif
