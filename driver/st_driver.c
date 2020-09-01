#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <video/mipi_display.h>

#include <linux/rmap.h>
#include <linux/pagemap.h>

#define MAX_INIT_SEQUENCE 512

#include "st.h"
#include "st_io.h"
#include "st_display.h"

static void st_set_addr_win(struct st_param *par, int xs, int ys,
		int xe, int ye);

static int st_write_spi(struct st_param *par, void *buf, size_t len)
{
	struct spi_transfer t = {
		.tx_buf = buf,
		.len = len,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(par->spi, &m);
}

static int st_write_buf_dc(struct st_param *par, void *buf, size_t len, int dc)
{
	if (gpio_is_valid(par->gpio.dc))
		gpio_set_value(par->gpio.dc, dc);

	return st_write_spi(par, buf, len);
}

static int st_write_vmem(struct st_param *par)
{
	int i;
	struct vm_pages *vp = &par->vm_pages;

	/* Send data */
	if (par->gpio.dc != -1)
		gpio_set_value(par->gpio.dc, 1);

	for (i = 0; i < vp->nr_pages; i++)
		st_write_spi(par, (void *)(vp->vm_pages[i]), PAGE_SIZE);

	return 0;
}

static void st_update_display(struct st_param *param, struct update *update)
{
	int x_start = update->xs;
	int y_start = update->ys;
	int x_end = update->xe;
	int y_end = update->ye;

	st_set_addr_win(param, x_start, y_start,
			x_end, y_end);

	st_write_vmem(param);
}

static int st_write_cmd(struct st_param *par, int len, u8 *buf)
{
	int ret;

	/* Send command */
	ret = st_write_buf_dc(par, buf, 1, 0);
	if (ret < 0) {
		pr_err("%s: command error\n", __func__);
		return -EINVAL;
	}
	if (!--len)
		return 0;
	/* Send data */
	ret = st_write_buf_dc(par, ++buf, len, 1);
	if (ret < 0)
		pr_err("%s: data error\n", __func__);

	return ret;
}

static int st_reset(struct st_param *par)
{
	if (par->gpio.reset == -1) {
		pr_err("reset error\n");
		return -EINVAL;
	}
	gpio_set_value_cansleep(par->gpio.reset, 0);
	usleep_range(20, 40);
	gpio_set_value_cansleep(par->gpio.reset, 1);
	msleep(120);
	return 0;
}

static int st_init_display(struct st_param *par)
{
	int ret;
	u8 buf[64];
	int i = 0;
	int j;

	ret = st_reset(par);
	if (ret)
		return ret;

	while (i < MAX_INIT_SEQUENCE) {
		if (par->init_sequence[i] == -3)
			return 0;

		switch (par->init_sequence[i]) {
		case -1:
			i++;
			j = 0;
			while (par->init_sequence[i] >= 0) {
				if (j > 63)
					return -EINVAL;
				buf[j++] = par->init_sequence[i++];
			}

			st_write_cmd(par, j, buf);
			break;
		case -2:
			i++	;
			mdelay(par->init_sequence[i++]);
			break;
		default:
			return -EINVAL;
		}
	}
	return -EINVAL;
}

static int st_request_one_gpio(struct st_param *par,
		const char *name, int index, int *gpiop)
{
	struct device *dev = par->device;
	struct device_node *node = dev->of_node;
	int gpio, flags, ret = 0;
	enum of_gpio_flags of_flags;

	if (of_find_property(node, name, NULL)) {
		gpio = of_get_named_gpio_flags(node, name, index, &of_flags);
		if (gpio == -ENOENT)
			return 0;
		if (gpio < 0)
			return gpio;
		flags = (of_flags & OF_GPIO_ACTIVE_LOW) ? GPIOF_OUT_INIT_LOW :
					GPIOF_OUT_INIT_HIGH;
		ret = devm_gpio_request_one(dev, gpio, flags,
				dev->driver->name);
		if (ret)
			return ret;
		if (gpiop)
			*gpiop = gpio;
	}

	return ret;
}

static int st_request_gpios(struct st_param *par)
{
	int ret;

	ret = st_request_one_gpio(par, "reset-gpios", 0, &par->gpio.reset);
	if (ret)
		return ret;
	ret = st_request_one_gpio(par, "dc-gpios", 0, &par->gpio.dc);
	if (ret)
		return ret;
	return 0;
}

static void st_set_addr_win(struct st_param *par, int xs, int ys,
		int xe, int ye)
{
	u8 buf[8];

	pr_debug("%s: start=%d:%d end=%d:%d\n", __func__,
			xs, ys, xe, ye);

	buf[0] = MIPI_DCS_SET_COLUMN_ADDRESS;
	buf[1] = (xs >> 8) & 0xFF;
	buf[2] = xs & 0xFF;
	buf[3] = (xe >> 8) & 0xFF;
	buf[4] = xe & 0xFF;
	st_write_cmd(par, 5, buf);

	buf[0] = MIPI_DCS_SET_PAGE_ADDRESS;
	buf[1] = (ys >> 8) & 0xFF;
	buf[2] = ys & 0xFF;
	buf[3] = (ye >> 8) & 0xFF;
	buf[4] = ye & 0xFF;
	st_write_cmd(par, 5, buf);

	buf[0] = MIPI_DCS_WRITE_MEMORY_START;
	st_write_cmd(par, 1, buf);
}

static int allocate_vm_pages(unsigned long *vm_pages, int nr_pages)
{
	int i;
	unsigned long page;

	for (i = 0; i < nr_pages; i++) {
		page = get_zeroed_page(GFP_KERNEL);
		if (!page)
			goto alloc_fail;
		pr_debug("%s: page=%lX\n", __func__, page);
		vm_pages[i] = page;
	}

	return 0;

alloc_fail:
	/* TODO: fix possible memleak */
	return -ENOMEM;
}

static void free_vm_pages(struct vm_pages *vm_pages)
{
	int i;
	unsigned long page;
	struct page *p;

	for (i = 0; i < vm_pages->nr_pages; i++) {
		page = vm_pages->vm_pages[i];
		p = virt_to_page((void *)page);
		pr_debug("%d: freeing page %lX, mapping=%lX\n",
				i, (unsigned long)page, (unsigned long)(p->mapping));
		p->mapping = NULL;
		free_page(page);
	}
	vfree(vm_pages->vm_pages);
}

struct st_param *st_param_alloc(struct st_display *display,
			struct device *dev)
{
	struct st_param *param;
	unsigned int width = display->width;
	unsigned int height = display->height;
	unsigned int bpp = 16;
	int vmem_size;
	int nr_pages;
	unsigned long *vm_pages;

	pr_debug("width=%u height=%d bpp=%d\n", width, height, bpp);

	vmem_size = width * height * bpp / 8;
	vmem_size = PAGE_ALIGN(vmem_size);

	nr_pages = vmem_size >> PAGE_SHIFT;
	vm_pages = vzalloc(nr_pages * (sizeof(unsigned long)));
	if (vm_pages == NULL)
		return NULL;

	pr_debug("%s: vmem size=%d nr_pages=%d", __func__,
			vmem_size, nr_pages);

	allocate_vm_pages(vm_pages, nr_pages);

	param = kzalloc(sizeof(struct st_param), GFP_KERNEL);

	strncpy(param->id, dev->driver->name, 16);
	param->line_length = width * bpp / 8;
	param->xres = width;
	param->yres = height;
	param->bits_per_pixel = bpp;

	param->red.offset = 11;
	param->red.length = 5;
	param->green.offset = 5;
	param->green.length = 6;
	param->blue.offset = 0;
	param->blue.length = 5;

	param->init_sequence = display->init_sequence;
	param->device = dev;

	/* Initialize gpios to disabled */
	param->gpio.reset = -1;
	param->gpio.dc = -1;

	param->vm_pages.vm_pages = vm_pages;
	param->vm_pages.nr_pages = nr_pages;

	return param;
}

static int st_mmap(struct file *file, struct vm_area_struct *vma)
{
	int i;
	unsigned long pfn;
	unsigned long page;
	unsigned long start = vma->vm_start;
	unsigned long size = (vma->vm_end - vma->vm_start);
	struct miscdevice *mdev = file->private_data;
	struct st_param *param = container_of(mdev, struct st_param, mdev);

	vma->vm_private_data = param;

	for (i = 0; i < param->vm_pages.nr_pages; i++) {
		page = param->vm_pages.vm_pages[i];
		pfn = virt_to_phys((void *)page) >> PAGE_SHIFT;
		if (remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED))
			return -EAGAIN;
		pr_debug("%lx (%lu) is mapped to %lx\n", page, pfn, start);
		start += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	return 0;
}

static long st_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct miscdevice *mdev = file->private_data;
	struct st_param *param = container_of(mdev, struct st_param, mdev);
	struct screeninfo si;
	struct update update;

	si.xres = param->xres;
	si.yres = param->yres;
	si.bpp = param->bits_per_pixel;
	si.line_length = param->line_length;

	switch (cmd) {
	case STGET_SCREENINFO:
		ret = copy_to_user((void *)arg, &si, sizeof(si));
		break;
	case ST_UPDATE:
		ret = copy_from_user((void *)&update, (const void __user *)arg, sizeof(update));
		pr_debug("%d %d %d %d\n", update.xs, update.ys, update.xe, update.ye);
		st_update_display(param, &update);
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static const struct file_operations st_fops = {
	.mmap = st_mmap,
	.unlocked_ioctl = st_ioctl,
};

static int st_register(struct st_param *st_param)
{
	int ret;
	struct spi_device *spi = st_param->spi;
	struct miscdevice *st_mdev = &st_param->mdev;
	struct update update = {
		.xs = 0,
		.ys = 0,
		.xe = st_param->xres - 1,
		.ye = st_param->yres - 1,
	};

	spi_set_drvdata(spi, st_param);

	ret = st_request_gpios(st_param);
	if (ret < 0)
		pr_err("Request gpios failed ret=%d\n", ret);

	ret = st_init_display(st_param);
	if (ret < 0)
		pr_err("Display init error ret=%d\n", ret);

	st_mdev->minor = MISC_DYNAMIC_MINOR;
	st_mdev->name = "st";
	st_mdev->fops = &st_fops;

	ret = misc_register(st_mdev);
	if (ret < 0)
		pr_err("Can't register misc device ret=%d\n", ret);

	st_update_display(st_param, &update);

	return 0;
}

static int st_probe_common(struct spi_device *sdev,
		struct st_display *display)
{
	struct device *dev;
	struct st_param *st_param;

	dev = &sdev->dev;

	st_param = st_param_alloc(display, dev);
	if (!st_param)
		return -ENOMEM;

	st_param->spi = sdev;
	st_register(st_param);

	return 0;
}

static int st_remove_common(struct device *dev, struct st_param *st_param)
{
	misc_deregister(&st_param->mdev);
	free_vm_pages(&st_param->vm_pages);
	kfree(st_param);
	return 0;
}

static int st_driver_probe_spi(struct spi_device *spi)
{
	return st_probe_common(spi, &display);
}

static int st_driver_remove_spi(struct spi_device *spi)
{
	struct st_param *param = spi_get_drvdata(spi);
	return st_remove_common(&spi->dev, param);
}

static struct spi_driver st_driver_spi_driver = {
	.probe = st_driver_probe_spi,
	.remove = st_driver_remove_spi,
};

static int __init st_driver_module_init(void)
{
	init_display(&st_driver_spi_driver.driver);
	return spi_register_driver(&st_driver_spi_driver);
}

static void __exit st_driver_module_exit(void)
{
	spi_unregister_driver(&st_driver_spi_driver);
}

module_init(st_driver_module_init);
module_exit(st_driver_module_exit);

MODULE_LICENSE("GPL");
