/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#include <common.h>
#include <mmc.h>
#include <pwm.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include <platform.h>
#include <mach-api.h>
#include <rtc_nxp.h>
#include <pm.h>

#include <draw_lcd.h>

#if defined(CONFIG_PMIC_NXE2000)
#include <power/pmic.h>
#include <nxe2000-private.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#include "eth.c"

/* add by cym 20151127 */
//Modified by Picsell Dois, for support of 640x480 screen
//int CFG_DISP_PRI_RESOL_WIDTH = 1024;
//int CFG_DISP_PRI_RESOL_HEIGHT = 768;
int cfg_disp_output_modole = 0;

int cfg_disp_pri_screen_layer = 0;
int cfg_disp_pri_screen_rgb_format = MLC_RGBFMT_A8R8G8B8;
int cfg_disp_pri_screen_pixel_byte = 4;
int cfg_disp_pri_screen_color_key = 0x090909;

int cfg_disp_pri_video_priority = 2;	// 0, 1, 2, 3;
int cfg_disp_pri_back_ground_color = 0x000000;

int cfg_disp_pri_mlc_interlace = CFALSE;

int cfg_disp_pri_resol_width = 1024;//800;
int cfg_disp_pri_resol_height = 768;//1280;

int cfg_disp_pri_hsync_sync_width = 20;
int cfg_disp_pri_hsync_back_porch= 160;
int cfg_disp_pri_hsync_front_porch = 160;
int cfg_disp_pri_hsync_active_high= CTRUE;

int cfg_disp_pri_vsync_sync_width = 3;
int cfg_disp_pri_vsync_back_porch = 23;
int cfg_disp_pri_vsync_front_porch = 12;
int cfg_disp_pri_vsync_active_high = CTRUE;

int cfg_disp_pri_clkgen0_source = DPC_VCLK_SRC_PLL2;
int cfg_disp_pri_clkgen0_div = 12; // even divide;
int cfg_disp_pri_clkgen0_delay = 0;
int cfg_disp_pri_clkgen0_invert = 0;
int cfg_disp_pri_clkgen1_source = DPC_VCLK_SRC_VCLK2;
int cfg_disp_pri_clkgen1_div = 1;
int cfg_disp_pri_clkgen1_delay = 0;
int cfg_disp_pri_clkgen1_invert = 0;
int cfg_disp_pri_clksel1_select = 0;
int cfg_disp_pri_padclksel = DPC_PADCLKSEL_VCLK;	/* VCLK=CLKGEN1, VCLK12=CLKGEN0 */;

int cfg_disp_pri_pixel_clock = 800000000/12;

int cfg_disp_pri_out_swaprb = CFALSE;
int cfg_disp_pri_out_format = DPC_FORMAT_RGB666;
int cfg_disp_pri_out_ycorder = DPC_YCORDER_CbYCrY;
int cfg_disp_pri_out_interlace = CFALSE;
int cfg_disp_pri_out_invert_field = CFALSE;

int cfg_disp_lvds_lcd_format = LVDS_LCDFORMAT_JEIDA;
/* end add */

#if (0)
#define DBGOUT(msg...)		{ printf("BD: " msg); }
#else
#define DBGOUT(msg...)		do {} while (0)
#endif

/*------------------------------------------------------------------------------
 * BUS Configure
 */
#if (CFG_BUS_RECONFIG_ENB == 1)
#include <asm/arch/s5p4418_bus.h>

const u16 g_DrexQoS[2] = {
	0x100,		// S0
	0xFFF		// S1, Default value
};

const u8 g_TopBusSI[8] = {
	TOPBUS_SI_SLOT_DMAC0,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST0,
	TOPBUS_SI_SLOT_DMAC1,
	TOPBUS_SI_SLOT_SDMMC,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST1,
	TOPBUS_SI_SLOT_USBOTG
};

const u8 g_BottomBusSI[8] = {
	BOTBUS_SI_SLOT_1ST_ARM,
	BOTBUS_SI_SLOT_MALI,
	BOTBUS_SI_SLOT_DEINTERLACE,
	BOTBUS_SI_SLOT_1ST_CODA,
	BOTBUS_SI_SLOT_2ND_ARM,
	BOTBUS_SI_SLOT_SCALER,
	BOTBUS_SI_SLOT_TOP,
	BOTBUS_SI_SLOT_2ND_CODA
};

#if 0
// default
const u8 g_BottomQoSSI[2] = {
	1,	// Tidemark
	(1<<BOTBUS_SI_SLOT_1ST_ARM) |	// Control
	(1<<BOTBUS_SI_SLOT_2ND_ARM) |
	(1<<BOTBUS_SI_SLOT_MALI) |
	(1<<BOTBUS_SI_SLOT_TOP) |
	(1<<BOTBUS_SI_SLOT_DEINTERLACE) |
	(1<<BOTBUS_SI_SLOT_1ST_CODA)
};
#else
const u8 g_BottomQoSSI[2] = {
	1,	// Tidemark
	(1<<BOTBUS_SI_SLOT_TOP)	// Control
};
#endif

const u8 g_DispBusSI[3] = {
	DISBUS_SI_SLOT_1ST_DISPLAY,
	DISBUS_SI_SLOT_2ND_DISPLAY,
	DISBUS_SI_SLOT_GMAC
};
#endif	/* #if (CFG_BUS_RECONFIG_ENB == 1) */

/*------------------------------------------------------------------------------
 * intialize nexell soc and board status.
 */
static void set_gpio_strenth(U32 Group, U32 BitNumber, U32 mA)
{
	U32 drv1=0, drv0=0;
	U32 drv1_value, drv0_value;

	switch( mA )
	{
		case 0 : drv0 = 0; drv1 = 0; break;
		case 1 : drv0 = 0; drv1 = 1; break;
		case 2 : drv0 = 1; drv1 = 0; break;
		case 3 : drv0 = 1; drv1 = 1; break;
		default: drv0 = 0; drv1 = 0; break;
	}
	DBGOUT("DRV Strength : GRP : i %x Bit: %x  ma :%d  \n",
				Group, BitNumber, mA);

	drv1_value = NX_GPIO_GetDRV1(Group) & ~(1 << BitNumber);
	drv0_value = NX_GPIO_GetDRV0(Group) & ~(1 << BitNumber);

	if (drv1) drv1_value |= (drv1 << BitNumber);
	if (drv0) drv0_value |= (drv0 << BitNumber);

	DBGOUT(" Value : drv1 :%8x  drv0 %8x \n ",drv1_value, drv0_value);

	NX_GPIO_SetDRV0 ( Group, drv0_value );
	NX_GPIO_SetDRV1 ( Group, drv1_value );
}

static void bd_gpio_init(void)
{
	int index, bit;
	int mode, func, out, lv, plup, stren;
	U32 gpio;

	const U32 pads[NUMBER_OF_GPIO_MODULE][32] = {
	{	/* GPIO_A */
	PAD_GPIOA0 , PAD_GPIOA1 , PAD_GPIOA2 , PAD_GPIOA3 , PAD_GPIOA4 , PAD_GPIOA5 , PAD_GPIOA6 , PAD_GPIOA7 , PAD_GPIOA8 , PAD_GPIOA9 ,
	PAD_GPIOA10, PAD_GPIOA11, PAD_GPIOA12, PAD_GPIOA13, PAD_GPIOA14, PAD_GPIOA15, PAD_GPIOA16, PAD_GPIOA17, PAD_GPIOA18, PAD_GPIOA19,
	PAD_GPIOA20, PAD_GPIOA21, PAD_GPIOA22, PAD_GPIOA23, PAD_GPIOA24, PAD_GPIOA25, PAD_GPIOA26, PAD_GPIOA27, PAD_GPIOA28, PAD_GPIOA29,
	PAD_GPIOA30, PAD_GPIOA31
	}, { /* GPIO_B */
	PAD_GPIOB0 , PAD_GPIOB1 , PAD_GPIOB2 , PAD_GPIOB3 , PAD_GPIOB4 , PAD_GPIOB5 , PAD_GPIOB6 , PAD_GPIOB7 , PAD_GPIOB8 , PAD_GPIOB9 ,
	PAD_GPIOB10, PAD_GPIOB11, PAD_GPIOB12, PAD_GPIOB13, PAD_GPIOB14, PAD_GPIOB15, PAD_GPIOB16, PAD_GPIOB17, PAD_GPIOB18, PAD_GPIOB19,
	PAD_GPIOB20, PAD_GPIOB21, PAD_GPIOB22, PAD_GPIOB23, PAD_GPIOB24, PAD_GPIOB25, PAD_GPIOB26, PAD_GPIOB27, PAD_GPIOB28, PAD_GPIOB29,
	PAD_GPIOB30, PAD_GPIOB31
	}, { /* GPIO_C */
	PAD_GPIOC0 , PAD_GPIOC1 , PAD_GPIOC2 , PAD_GPIOC3 , PAD_GPIOC4 , PAD_GPIOC5 , PAD_GPIOC6 , PAD_GPIOC7 , PAD_GPIOC8 , PAD_GPIOC9 ,
	PAD_GPIOC10, PAD_GPIOC11, PAD_GPIOC12, PAD_GPIOC13, PAD_GPIOC14, PAD_GPIOC15, PAD_GPIOC16, PAD_GPIOC17, PAD_GPIOC18, PAD_GPIOC19,
	PAD_GPIOC20, PAD_GPIOC21, PAD_GPIOC22, PAD_GPIOC23, PAD_GPIOC24, PAD_GPIOC25, PAD_GPIOC26, PAD_GPIOC27, PAD_GPIOC28, PAD_GPIOC29,
	PAD_GPIOC30, PAD_GPIOC31
	}, { /* GPIO_D */
	PAD_GPIOD0 , PAD_GPIOD1 , PAD_GPIOD2 , PAD_GPIOD3 , PAD_GPIOD4 , PAD_GPIOD5 , PAD_GPIOD6 , PAD_GPIOD7 , PAD_GPIOD8 , PAD_GPIOD9 ,
	PAD_GPIOD10, PAD_GPIOD11, PAD_GPIOD12, PAD_GPIOD13, PAD_GPIOD14, PAD_GPIOD15, PAD_GPIOD16, PAD_GPIOD17, PAD_GPIOD18, PAD_GPIOD19,
	PAD_GPIOD20, PAD_GPIOD21, PAD_GPIOD22, PAD_GPIOD23, PAD_GPIOD24, PAD_GPIOD25, PAD_GPIOD26, PAD_GPIOD27, PAD_GPIOD28, PAD_GPIOD29,
	PAD_GPIOD30, PAD_GPIOD31
	}, { /* GPIO_E */
	PAD_GPIOE0 , PAD_GPIOE1 , PAD_GPIOE2 , PAD_GPIOE3 , PAD_GPIOE4 , PAD_GPIOE5 , PAD_GPIOE6 , PAD_GPIOE7 , PAD_GPIOE8 , PAD_GPIOE9 ,
	PAD_GPIOE10, PAD_GPIOE11, PAD_GPIOE12, PAD_GPIOE13, PAD_GPIOE14, PAD_GPIOE15, PAD_GPIOE16, PAD_GPIOE17, PAD_GPIOE18, PAD_GPIOE19,
	PAD_GPIOE20, PAD_GPIOE21, PAD_GPIOE22, PAD_GPIOE23, PAD_GPIOE24, PAD_GPIOE25, PAD_GPIOE26, PAD_GPIOE27, PAD_GPIOE28, PAD_GPIOE29,
	PAD_GPIOE30, PAD_GPIOE31
	},
	};

	/* GPIO pad function */
	for (index = 0; NUMBER_OF_GPIO_MODULE > index; index++) {

		NX_GPIO_ClearInterruptPendingAll(index);

		for (bit = 0; 32 > bit; bit++) {
			gpio  = pads[index][bit];
			func  = PAD_GET_FUNC(gpio);
			mode  = PAD_GET_MODE(gpio);
			lv    = PAD_GET_LEVEL(gpio);
			stren = PAD_GET_STRENGTH(gpio);
			plup  = PAD_GET_PULLUP(gpio);

			/* get pad alternate function (0,1,2,4) */
			switch (func) {
			case PAD_GET_FUNC(PAD_FUNC_ALT0): func = NX_GPIO_PADFUNC_0;	break;
			case PAD_GET_FUNC(PAD_FUNC_ALT1): func = NX_GPIO_PADFUNC_1;	break;
			case PAD_GET_FUNC(PAD_FUNC_ALT2): func = NX_GPIO_PADFUNC_2;	break;
			case PAD_GET_FUNC(PAD_FUNC_ALT3): func = NX_GPIO_PADFUNC_3;	break;
			default: printf("ERROR, unknown alt func (%d.%02d=%d)\n", index, bit, func);
				continue;
			}

			switch (mode) {
			case PAD_GET_MODE(PAD_MODE_ALT): out = 0;
			case PAD_GET_MODE(PAD_MODE_IN ): out = 0;
			case PAD_GET_MODE(PAD_MODE_INT): out = 0; break;
			case PAD_GET_MODE(PAD_MODE_OUT): out = 1; break;
			default: printf("ERROR, unknown io mode (%d.%02d=%d)\n", index, bit, mode);
				continue;
			}

			NX_GPIO_SetPadFunction(index, bit, func);
			NX_GPIO_SetOutputEnable(index, bit, (out ? CTRUE : CFALSE));
			NX_GPIO_SetOutputValue(index, bit,  (lv  ? CTRUE : CFALSE));
			NX_GPIO_SetInterruptMode(index, bit, (lv));

			NX_GPIO_SetPullMode(index, bit, plup);
			set_gpio_strenth(index, bit, stren); /* pad strength */
		}

		NX_GPIO_SetDRV0_DISABLE_DEFAULT(index, 0xFFFFFFFF);
		NX_GPIO_SetDRV1_DISABLE_DEFAULT(index, 0xFFFFFFFF);
	}
}

static void bd_alive_init(void)
{
	int index, bit;
	int mode, out, lv, plup, detect;
	U32 gpio;

	const U32 pads[] = {
	PAD_GPIOALV0, PAD_GPIOALV1, PAD_GPIOALV2,
	PAD_GPIOALV3, PAD_GPIOALV4, PAD_GPIOALV5
	};

	index = sizeof(pads)/sizeof(pads[0]);

	/* Alive pad function */
	for (bit = 0; index > bit; bit++) {
		NX_ALIVE_ClearInterruptPending(bit);
		gpio = pads[bit];
		mode = PAD_GET_MODE(gpio);
		lv   = PAD_GET_LEVEL(gpio);
		plup = PAD_GET_PULLUP(gpio);

		switch (mode) {
		case PAD_GET_MODE(PAD_MODE_IN ):
		case PAD_GET_MODE(PAD_MODE_INT): out = 0; break;
		case PAD_GET_MODE(PAD_MODE_OUT): out = 1; break;
		case PAD_GET_MODE(PAD_MODE_ALT):
			printf("ERROR, alive.%d not support alt function\n", bit);
			continue;
		default :
			printf("ERROR, unknown alive mode (%d=%d)\n", bit, mode);
			continue;
		}

		NX_ALIVE_SetOutputEnable(bit, (out ? CTRUE : CFALSE));
		NX_ALIVE_SetOutputValue (bit, (lv));
		NX_ALIVE_SetPullUpEnable(bit, (plup & 1 ? CTRUE : CFALSE));
		/* set interrupt mode */
		for (detect = 0; 6 > detect; detect++) {
			if (mode == PAD_GET_MODE(PAD_MODE_INT))
				NX_ALIVE_SetDetectMode(detect, bit, (lv == detect ? CTRUE : CFALSE));
			else
				NX_ALIVE_SetDetectMode(detect, bit, CFALSE);
		}
		NX_ALIVE_SetDetectEnable(bit, (mode == PAD_MODE_INT ? CTRUE : CFALSE));
	}
}

/* call from u-boot */
int board_early_init_f(void)
{
	bd_gpio_init();
	bd_alive_init();
#if (defined(CONFIG_PMIC_NXE2000)||defined(CONFIG_PMIC_AXP228))&& !defined(CONFIG_PMIC_REG_DUMP)
	bd_pmic_init();
#endif
#if defined(CONFIG_NXP_RTC_USE)
	nxp_rtc_init();
#endif

	return 0;
}

int board_init(void)
{
	//Modification: Beep when powering up, added by Picsell Dois (T.C.D.)
	gpio_direction_output((PAD_GPIO_C + 14), 1);
	mdelay(500);
	gpio_direction_output((PAD_GPIO_C + 14), 0);
	//End-of-Modification: Beep when powering up, added by Picsell Dois (T.C.D.)

	DBGOUT("%s : done board init ...\n", CFG_SYS_BOARD_NAME);
	return 0;
}

#if defined(CONFIG_PMIC_NXE2000)||defined(CONFIG_PMIC_AXP228)
int power_init_board(void)
{
	int ret = 0;
#if defined(CONFIG_PMIC_REG_DUMP)
	bd_pmic_init();
#endif
	ret = power_pmic_function_init();
	return ret;
}
#endif

extern void	bd_display(void);

/* add by cym 20180409 */
#if 1
static void auto_fastboot(int io, int wait)
{
        unsigned int grp = PAD_GET_GROUP(io);
        unsigned int bit = PAD_GET_BITNO(io);
        int level = 1, i = 0;

	char *cmd = "sdfuse flashall";

        for (i = 0; wait > i; i++)
        {
		switch (io & ~(32-1)) {
                case PAD_GPIO_A:
                case PAD_GPIO_B:
                case PAD_GPIO_C:
                case PAD_GPIO_D:
                case PAD_GPIO_E:
                        level = NX_GPIO_GetInputValue(grp, bit);        break;
                case PAD_GPIO_ALV:
                        level = NX_ALIVE_GetInputValue(bit);    break;
                };
                if (level)
                        break;
                mdelay(1);
	}

	if(i == wait)
        {
        	printf("auto fastboot,start ...\n");

                gpio_direction_output((PAD_GPIO_C + 14), 1);
                mdelay(50);
                gpio_direction_output((PAD_GPIO_C + 14), 0);

		run_command (cmd, 0);
	}
	else
		printf("Normal boot ...\n");
}
#endif
/* end add */

static void auto_update(int io, int wait)
{
	unsigned int grp = PAD_GET_GROUP(io);
	unsigned int bit = PAD_GET_BITNO(io);
	int level = 1, i = 0;
	char *cmd = "fastboot";

	for (i = 0; wait > i; i++) {
		switch (io & ~(32-1)) {
		case PAD_GPIO_A:
		case PAD_GPIO_B:
		case PAD_GPIO_C:
		case PAD_GPIO_D:
		case PAD_GPIO_E:
			level = NX_GPIO_GetInputValue(grp, bit);	break;
		case PAD_GPIO_ALV:
			level = NX_ALIVE_GetInputValue(bit);	break;
		};
		if (level)
			break;
		mdelay(1);
	}

	if (i == wait)
		run_command (cmd, 0);
}

void bd_display_run(char *cmd, int bl_duty, int bl_on)
{
	static int display_init = 0;

	if (cmd) {
		run_command(cmd, 0);
		lcd_draw_boot_logo(CONFIG_FB_ADDR, cfg_disp_pri_resol_width,
			cfg_disp_pri_resol_height, cfg_disp_pri_screen_pixel_byte);
	}

	if (!display_init) {
		bd_display();
		pwm_init(CFG_LCD_PRI_PWM_CH, 0, 0);
		display_init = 1;
	}

	pwm_config(CFG_LCD_PRI_PWM_CH,
		TO_DUTY_NS(bl_duty, CFG_LCD_PRI_PWM_FREQ),
		TO_PERIOD_NS(CFG_LCD_PRI_PWM_FREQ));

	if (bl_on)
		pwm_enable(CFG_LCD_PRI_PWM_CH);
}

#define	UPDATE_KEY			(PAD_GPIO_ALV + 0)
#define	UPDATE_CHECK_TIME	(3000)	/* ms */

/* add by cym 20180409 */
#define AUTO_FASTBOOT_KEY                      (PAD_GPIO_A + 28)	//back key
/* end add */

int board_late_init(void)
{
	/* add by cym 20151127 */
	char *p = NULL;
	char bootargs[200] = {0};
	/* end add */

	/* add by cym 20160107 */
	char boot_system_flags = 0;	//0:android, 1:QT, 2:Ubuntu
	/* end add */

	/* add by cym 20160830 */
	char android_version = 4;
	/* end add */

	/* add by cym 20171023 */
	#define CFG_FASTBOOT_SDFUSE_MMCDEV      0

	char tf_insert_flags = 0;

	struct mmc *mmc = find_mmc_device(CFG_FASTBOOT_SDFUSE_MMCDEV);
	if (mmc_init(mmc)) {
		printf("Not found tf card.\n");
	
		tf_insert_flags = 0;
	}
	else
	{
		printf("Found tf card.\n");

		tf_insert_flags = 1;
	}
	/* end add */
	
#if defined(CONFIG_SYS_MMC_BOOT_DEV)
	char boot[16];
	sprintf(boot, "mmc dev %d", CONFIG_SYS_MMC_BOOT_DEV);
	run_command(boot, 0);
#endif

#if defined(CONFIG_RECOVERY_BOOT)
    if (RECOVERY_SIGNATURE == readl(SCR_RESET_SIG_READ)) {
        writel((-1UL), SCR_RESET_SIG_RESET); /* clear */

        printf("RECOVERY BOOT\n");
        bd_display_run(CONFIG_CMD_LOGO_WALLPAPERS, CFG_LCD_PRI_PWM_DUTYCYCLE, 1);
        run_command(CONFIG_CMD_RECOVERY_BOOT, 0);	/* recovery boot */
    }
    writel((-1UL), SCR_RESET_SIG_RESET);
#endif /* CONFIG_RECOVERY_BOOT */

#if defined(CONFIG_BAT_CHECK)
	{
		int ret =0;
		int bat_check_skip = 0;
	    // psw0523 for cts
	     bat_check_skip = 1; //turn on by cym 20150811 to skip check battery

		/* add by cym 20161125 for 4G Module reset */
		gpio_direction_output((PAD_GPIO_A + 30), 1);
		mdelay(200);
		gpio_direction_output((PAD_GPIO_A + 30), 0);
		/* end add */

		/* add by cym 20171023 */
		p = getenv("bootargs");
		//Modified by Picsell Dois, for support of 640x480 screen
		if(1){
		/* end add */

		/* add by cym 20160107 */
		p = getenv("bootsystem");
		if (NULL == p) {
                        printf("*** Warning use default bootsystem:%s ***\n", CONFIG_BOOT_SYSTEM);
                        p = CONFIG_BOOT_SYSTEM;

                        setenv("bootsystem", (char *)p);
                        saveenv();
                }

		printf("Boot system :%s\n", p);
		if(!strcmp(p, "android"))
		{
			p = getenv("androidversion");
			if(NULL == p)
			{
				printf("*** Warning use default androidversion:%s ***\n", CONFIG_ANDROID_VERSION);
				p = CONFIG_ANDROID_VERSION;

				setenv("androidversion", (char *)p);
				saveenv();
			}

			printf("android version:%s\n", p);
			if(!strcmp(p, "4.4"))
			{
				android_version = 4;
			}
			else if(!strcmp(p, "5.1"))
			{
				android_version = 5;
			}
			else
			{
				android_version = 4;
			}

			boot_system_flags = 0;
		}
		else if(!strcmp(p, "qt"))
		{
			boot_system_flags = 1;
		}
		else if((!strcmp(p, "ubuntu_tf")) || (!strcmp(p, "ubuntu")))
		{
			boot_system_flags = 2;
		}
		else if(!strcmp(p, "ubuntu_emmc"))
		{
			boot_system_flags = 3;
		}
		/* end add */

		/* add by cym 20151127 */
		p = getenv("lcdtype");
		if (NULL == p) {
			printf("*** Warning use default lcdtype:%s ***\n", CONFIG_DISPLAY_LCD_TYPE);
			p = CONFIG_DISPLAY_LCD_TYPE;

			setenv("lcdtype", (char *)p);
			saveenv();
		}

		//p = getenv("lcdtype");
		printf("LCD type:%s\n", p);

		if(!strcmp(p, "9.7"))
		{
			cfg_disp_pri_resol_width = 1024;
			cfg_disp_pri_resol_height= 768;

			cfg_disp_pri_clkgen0_div = 14;
		}
		else if(!strcmp(p, "7.0"))
		{
			cfg_disp_pri_resol_width = 800;
			cfg_disp_pri_resol_height = 1280;

			cfg_disp_pri_hsync_back_porch = 48;
			cfg_disp_pri_hsync_front_porch = 16;

			cfg_disp_pri_vsync_back_porch = 3;
			cfg_disp_pri_vsync_front_porch = 5;
		}
#if 1
		else if(!strcmp(p, "4.3"))
		{
			//cfg_disp_pri_mlc_interlace = CTRUE;

			cfg_disp_pri_resol_width = 480;
                        cfg_disp_pri_resol_height= 272;

			cfg_disp_pri_clkgen0_div = 42;//12; // even divide;
			//cfg_disp_pri_clkgen1_div = 2;//1;
		}
#endif
		else if(!strcmp(p, "5.0"))
                {
                        //cfg_disp_pri_mlc_interlace = CTRUE;

                        cfg_disp_pri_resol_width = 800;
                        cfg_disp_pri_resol_height= 480;

                        cfg_disp_pri_clkgen0_div = 12;//24;//12; // even divide;
                        cfg_disp_pri_clkgen1_div = 1;//2;//1;
                }
		else if(!strcmp(p, "1024x600"))
                {
                        //cfg_disp_pri_mlc_interlace = CTRUE;

                        cfg_disp_pri_resol_width = 1024;
                        cfg_disp_pri_resol_height= 600;

			cfg_disp_pri_clkgen0_div = 15;
                        //cfg_disp_pri_clkgen0_div = 24;//12; // even divide;
                        //cfg_disp_pri_clkgen1_div = 2;//1;
                }
		else if(!strcmp(p, "hdmi"))
                {
                        cfg_disp_pri_resol_width = 1920;
                        cfg_disp_pri_resol_height= 1080;
                }
		//Modified by Picsell Dois, for support of 640x480 screen
		else if(!strcmp(p, "640x480"))
                {
                        cfg_disp_pri_resol_width = 640;
                        cfg_disp_pri_resol_height= 480;
                }
		else
		{
			cfg_disp_pri_resol_width = 1024;
			cfg_disp_pri_resol_height = 768;
		}

		if(0 == boot_system_flags)
		{
			if(4 == android_version)
			{
				sprintf(bootargs, "console=ttyAMA0,115200n8 androidboot.hardware=drone2 androidboot.console=ttyAMA0 androidboot.serialno=0123456789abcdef initrd=0x49000000,0x1000000 init=/init lcdtype=%s", p);
			}
			else if(5 == android_version)
			{
				sprintf(bootargs, "console=ttyAMA0,115200n8 androidboot.hardware=s5p4418_drone androidboot.console=ttyAMA0 androidboot.serialno=0123456789abcdef initrd=0x49000000,0x1000000 init=/init lcdtype=%s", p);
			}

			setenv("bootcmd", "ext4load mmc 2:1 0x48000000 uImage;ext4load mmc 2:1 0x49000000 root.img.gz;bootm 0x48000000");
		}
		else if(1 == boot_system_flags)//QT
		{
			if(!tf_insert_flags)
				sprintf(bootargs, "console=ttyAMA0,115200 root=/dev/mmcblk0p2 init=/linuxrc rootfstype=ext4 lcdtype=%s rootwait", p);
			else
				sprintf(bootargs, "console=ttyAMA0,115200 root=/dev/mmcblk1p2 init=/linuxrc rootfstype=ext4 lcdtype=%s rootwait", p);

			setenv("bootcmd", "ext4load mmc 2:1 0x48000000 uImage;bootm 0x48000000");
		}
		else if(2 == boot_system_flags)//Ubuntu TF
		{
			sprintf(bootargs, "console=ttyAMA0,115200 root=/dev/mmcblk0p1 init=/linuxrc rootfstype=ext4 lcdtype=%s bootsystem=%s rootwait", p, getenv("bootsystem"));

                        setenv("bootcmd", "ext4load mmc 2:1 0x48000000 uImage;bootm 0x48000000");
		}
		else if(3 == boot_system_flags)//Ubuntu EMMC
                {
			if(!tf_insert_flags)
                        	sprintf(bootargs, "console=ttyAMA0,115200 root=/dev/mmcblk0p7 init=/linuxrc rootfstype=ext4 lcdtype=%s bootsystem=%s rootwait", p, getenv("bootsystem"));
			else
				sprintf(bootargs, "console=ttyAMA0,115200 root=/dev/mmcblk1p7 init=/linuxrc rootfstype=ext4 lcdtype=%s bootsystem=%s rootwait", p, getenv("bootsystem"));

                        setenv("bootcmd", "ext4load mmc 2:1 0x48000000 uImage;bootm 0x48000000");
                }
		else
		{
			sprintf(bootargs, "console=ttyAMA0,115200n8 androidboot.hardware=drone2 androidboot.console=ttyAMA0 androidboot.serialno=0123456789abcdef initrd=0x49000000,0x1000000 init=/init lcdtype=%s", p);

                        setenv("bootcmd", "ext4load mmc 2:1 0x48000000 uImage;ext4load mmc 2:1 0x49000000 root.img.gz;bootm 0x48000000");
		}
		//printf("bootargs:%s\n", bootargs);
		setenv("bootargs", bootargs);
		/* end add */
	}

#if defined(CONFIG_DISPLAY_OUT)
		ret = power_battery_check(bat_check_skip, bd_display_run);
#else
		ret = power_battery_check(bat_check_skip, NULL);
#endif

		if(ret == 1)
			auto_update(UPDATE_KEY, UPDATE_CHECK_TIME);
		/*add by cym 20180409 */
		else
                	auto_fastboot(AUTO_FASTBOOT_KEY, 600);
                /* end add */
	}
#else /* CONFIG_BAT_CHECK */

#if defined(CONFIG_DISPLAY_OUT)
	bd_display_run(CONFIG_CMD_LOGO_WALLPAPERS, CFG_LCD_PRI_PWM_DUTYCYCLE, 1);
#endif

	/* Temp check gpio to update */
	auto_update(UPDATE_KEY, UPDATE_CHECK_TIME);

#endif /* CONFIG_BAT_CHECK */

	return 0;
}

