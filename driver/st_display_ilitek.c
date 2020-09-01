#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of.h>
#include <video/mipi_display.h>

#include "st_display.h"

#define DRVNAME "ilitek"

static const s16 default_init_sequence[] = {
	-1, MIPI_DCS_SOFT_RESET,
	-2, 5,
	-1, MIPI_DCS_SET_DISPLAY_OFF,
/* --------------------------------------------------------- */
	-1, 0xCF, 0x00, 0x83, 0x30,
	-1, 0xED, 0x64, 0x03, 0x12, 0x81,
	-1, 0xE8, 0x85, 0x01, 0x79,
	-1, 0xCB, 0x39, 0X2C, 0x00, 0x34, 0x02,
	-1, 0xF7, 0x20,
	-1, 0xEA, 0x00, 0x00,
/* ------------power control-------------------------------- */
	-1, 0xC0, 0x26,
	-1, 0xC1, 0x11,
   /* ------------VCOM --------- */
	-1, 0xC5, 0x35, 0x3E,
	-1, 0xC7, 0xBE,
/* ------------memory access control------------------------ */
	-1, MIPI_DCS_SET_PIXEL_FORMAT, 0x55, /* 16bit pixel */
/* ------------frame rate----------------------------------- */
	-1, 0xB1, 0x00, 0x1B,
/* ------------Gamma---------------------------------------- */
	/* -1, 0xF2, 0x08, */ /* Gamma Function Disable */
/*	-1, MIPI_DCS_SET_GAMMA_CURVE, 0x01,	*/
/* ------------display-------------------------------------- */
	-1, 0xB7, 0x07, /* entry mode set */
	//-1, 0xB6, 0x0A, 0x82, 0x27, 0x00,
	-1, 0xB6, 0x0A, 0xA2, 0x27, 0x00,
	-1, MIPI_DCS_EXIT_SLEEP_MODE,
	-2, 100,
	-1, MIPI_DCS_SET_DISPLAY_ON,
	-2, 20,
	-3
};

struct st_display display = {
	.regwidth = 8,
	.buswidth = 8,
	.width = 240,
	.height = 320,
	.init_sequence = default_init_sequence,
};

const struct of_device_id dt_ids[] = {
	{ .compatible = "ilitek,ili9341" },
	{},
};

MODULE_DEVICE_TABLE(of, dt_ids);

void init_display(struct device_driver *driver)
{
	driver->name = DRVNAME;
	driver->of_match_table = dt_ids;
}
