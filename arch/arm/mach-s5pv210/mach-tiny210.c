/* linux/arch/arm/mach-s5pv210/mach-tiny210.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/device.h>
#include <linux/dm9000.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/pwm_backlight.h>
#include <linux/platform_data/s3c-hsotg.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <video/platform_lcd.h>
#include <video/samsung_fimd.h>

#include <mach/map.h>
#include <mach/regs-clock.h>

#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <linux/platform_data/touchscreen-s3c2410.h>
#include <linux/platform_data/ata-samsung_cf.h>
#include <linux/platform_data/i2c-s3c2410.h>
#include <plat/keypad.h>
#include <plat/pm.h>
#include <plat/fb.h>
#include <plat/samsung-time.h>
#include <plat/backlight.h>
#include <plat/mfc.h>
#include <plat/clock.h>

#include "common.h"

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define TINY210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define TINY210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define TINY210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg tiny210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= TINY210_UCON_DEFAULT,
		.ulcon		= TINY210_ULCON_DEFAULT,
		.ufcon		= TINY210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= TINY210_UCON_DEFAULT,
		.ulcon		= TINY210_ULCON_DEFAULT,
		.ufcon		= TINY210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= TINY210_UCON_DEFAULT,
		.ulcon		= TINY210_ULCON_DEFAULT,
		.ufcon		= TINY210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= TINY210_UCON_DEFAULT,
		.ulcon		= TINY210_ULCON_DEFAULT,
		.ufcon		= TINY210_UFCON_DEFAULT,
	},
};

static struct resource tiny210_dm9000_resources[] = {
	[0] = DEFINE_RES_MEM(S5PV210_PA_SROM_BANK1 + 0x0000, 4),
	[1] = DEFINE_RES_MEM(S5PV210_PA_SROM_BANK1 + 0x0008, 4),
	[2] = DEFINE_RES_NAMED(IRQ_EINT(7), 1, NULL, IORESOURCE_IRQ \
				| IORESOURCE_IRQ_HIGHLEVEL),
};

static struct dm9000_plat_data tiny210_dm9000_platdata = {
	.flags		= DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM,
	.dev_addr	= { 0x12, 0x34, 0x56, 0x7A, 0xBC, 0xDE },
};

static struct platform_device tiny210_dm9000 = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(tiny210_dm9000_resources),
	.resource	= tiny210_dm9000_resources,
	.dev		= {
		.platform_data	= &tiny210_dm9000_platdata,
	},
};

static struct platform_device *tiny210_devices[] __initdata = {
	&s3c_device_adc,
	&s3c_device_cfcon,
	&s3c_device_fb,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_rtc,
	&s3c_device_ts,
	&s3c_device_wdt,
	&s5p_device_fimc0,
	&s5p_device_fimc1,
	&s5p_device_fimc2,
	&s5p_device_fimc_md,
	&s5p_device_jpeg,
	&s5p_device_mfc,
	&s5p_device_mfc_l,
	&s5p_device_mfc_r,
	&s5pv210_device_ac97,
	&s5pv210_device_iis0,
	&s5pv210_device_spdif,
	&tiny210_dm9000,
};

static void __init tiny210_dm9000_init(void)
{
	unsigned int tmp;

	gpio_request(S5PV210_MP01(1), "nCS1");
	s3c_gpio_cfgpin(S5PV210_MP01(1), S3C_GPIO_SFN(2));
	gpio_free(S5PV210_MP01(1));

	tmp = (5 << S5P_SROM_BCX__TACC__SHIFT);
	__raw_writel(tmp, S5P_SROM_BC1);

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= (S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS1__SHIFT);
	tmp |= (1 << S5P_SROM_BW__NCS1__SHIFT);
	__raw_writel(tmp, S5P_SROM_BW);
}

static struct i2c_board_info tiny210_i2c_devs0[] __initdata = {
	//{ I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
	//{ I2C_BOARD_INFO("wm8580", 0x1b), },
};

static struct i2c_board_info tiny210_i2c_devs1[] __initdata = {
	/* To Be Updated */
};

static struct i2c_board_info tiny210_i2c_devs2[] __initdata = {
	/* To Be Updated */
};

/* LCD Backlight data */
static struct samsung_bl_gpio_info tiny210_bl_gpio_info = {
	.no = S5PV210_GPD0(1),
	.func = S3C_GPIO_SFN(2),
};

static struct platform_pwm_backlight_data tiny210_bl_data = {
	.pwm_id = 1,
	.pwm_period_ns = 1000,
};

static void __init tiny210_map_io(void)
{
	s5pv210_init_io(NULL, 0);
	s3c24xx_init_clocks(clk_xusbxti.rate);
	s3c24xx_init_uarts(tiny210_uartcfgs, ARRAY_SIZE(tiny210_uartcfgs));
	samsung_set_timer_source(SAMSUNG_PWM2, SAMSUNG_PWM4);
}

static void __init tiny210_reserve(void)
{
	s5p_mfc_reserve_mem(0x43000000, 8 << 20, 0x51000000, 8 << 20);
}

static void __init tiny210_machine_init(void)
{
	s3c_pm_init();

	tiny210_dm9000_init();

	s3c24xx_ts_set_platdata(NULL);

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, tiny210_i2c_devs0,
			ARRAY_SIZE(tiny210_i2c_devs0));
	i2c_register_board_info(1, tiny210_i2c_devs1,
			ARRAY_SIZE(tiny210_i2c_devs1));
	i2c_register_board_info(2, tiny210_i2c_devs2,
			ARRAY_SIZE(tiny210_i2c_devs2));

	samsung_bl_set(&tiny210_bl_gpio_info, &tiny210_bl_data);

	platform_add_devices(tiny210_devices, ARRAY_SIZE(tiny210_devices));
}

MACHINE_START(TINY210, "TINY210")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.atag_offset	= 0x100,
	.init_irq	= s5pv210_init_irq,
	.map_io		= tiny210_map_io,
	.init_machine	= tiny210_machine_init,
	.init_time	= samsung_timer_init,
	.restart	= s5pv210_restart,
	.reserve	= &tiny210_reserve,
MACHINE_END
