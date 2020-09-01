#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of.h>
#include <video/mipi_display.h>

#include "st_display.h"

#define DRVNAME "st7735r"

static const s16 default_init_sequence[] = {
	-1, MIPI_DCS_SOFT_RESET,
	-2, 150,
	-1, MIPI_DCS_EXIT_SLEEP_MODE,
	-2, 500,
	-1, 0xB1, 0x01, 0x2C, 0x2D, /* FRMCTR1, Frame Rate Control in normal mode/full colors */
	-1, 0xC0, 0xA2, 0x02, 0x84,
	-1, 0xC1, 0xC5,
	-1, 0xC2, 0x0A, 0x00,
	-1, MIPI_DCS_EXIT_INVERT_MODE,
	-1, MIPI_DCS_SET_PIXEL_FORMAT, MIPI_DCS_PIXEL_FMT_16BIT,
	-1, MIPI_DCS_SET_DISPLAY_ON,
	-2, 100,
	-1, MIPI_DCS_ENTER_NORMAL_MODE,
	-2, 10,
	-3
};

struct st_display display = {
	.regwidth = 8,
	.buswidth = 8,
	.width = 128,
	.height = 160,
	.init_sequence = default_init_sequence,
};

const struct of_device_id dt_ids[] = {
	{ .compatible = "sitronix,st7735r" },
	{},
};

MODULE_DEVICE_TABLE(of, dt_ids);

void init_display(struct device_driver *driver)
{
	driver->name = DRVNAME;
	driver->of_match_table = dt_ids;
}
