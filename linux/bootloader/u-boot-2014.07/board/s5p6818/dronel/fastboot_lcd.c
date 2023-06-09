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
#include <pwm.h>
#include <platform.h>
#include <draw_lcd.h>
#include <asm/arch/fastboot.h>

/* add by cym 20151127 */
#include <cfg_common.h>
/* end add */

#define	LOGO_BGCOLOR	(0xffffff)
/* modify by cym 20151127 */
#if 0
static int _logo_left   = CFG_DISP_PRI_RESOL_WIDTH /2 +  50;
static int _logo_top    = CFG_DISP_PRI_RESOL_HEIGHT/2 + 180;
#else
static int _logo_left;
static int _logo_top;
#endif
/* end modify */
static int _logo_width  = 8*24;
static int _logo_height = 16;

void fboot_lcd_start(void)
{
	/* add by cym 20151127 */
	_logo_left = cfg_disp_pri_resol_width /2 +  50;
	_logo_top = cfg_disp_pri_resol_height/2 + 180;
	/* end add */
	
	lcd_info lcd = {
		.fb_base		= CONFIG_FB_ADDR,
		.bit_per_pixel	= cfg_disp_pri_screen_pixel_byte * 8,
		.lcd_width		= cfg_disp_pri_resol_width,
		.lcd_height		= cfg_disp_pri_resol_height,
		.back_color		= LOGO_BGCOLOR,
		.text_color		= 0xFF,
		.alphablend		= 0,
	};
	lcd_debug_init(&lcd);

	/* clear FB */
	memset((void*)CONFIG_FB_ADDR, 0xFF,
		cfg_disp_pri_resol_width * cfg_disp_pri_resol_height *
		cfg_disp_pri_screen_pixel_byte);

	run_command(CONFIG_CMD_LOGO_UPDATE, 0);
	lcd_draw_text("wait for update", _logo_left, _logo_top, 2, 2, 0);
}

void fboot_lcd_stop(void)
{
	run_command(CONFIG_CMD_LOGO_WALLPAPERS, 0);
}

void fboot_lcd_part(char *part, char *stat)
{
	/* add by cym 20151127 */
	_logo_left = cfg_disp_pri_resol_width /2 +  50;
	_logo_top = cfg_disp_pri_resol_height/2 + 180;
	/* end add */
	
	int s = 2;
	int l = _logo_left, t = _logo_top;
	int w = (_logo_width*s), h = (_logo_height*s);
	unsigned bg = LOGO_BGCOLOR;

	lcd_fill_rectangle(l, t, w, h, bg, 0);
	lcd_draw_string(l, t, s, s, 0, "%s: %s", part, stat);
}

void fboot_lcd_down(int percent)
{
	/* add by cym 20151127 */
	_logo_left = cfg_disp_pri_resol_width /2 +  50;
	_logo_top = cfg_disp_pri_resol_height/2 + 180;
	/* end add */
	
	int s = 2;
	int l = _logo_left, t = _logo_top;
	int w = (_logo_width*s), h = (_logo_height*s);
	unsigned bg = LOGO_BGCOLOR;

	lcd_fill_rectangle(l, t, w, h, bg, 0);
	lcd_draw_string(l, t, s, s, 0, "down %d%%", percent);
}

void fboot_lcd_flash(char *part, char *stat)
{
	/* add by cym 20151127 */
	_logo_left = cfg_disp_pri_resol_width /2 +  50;
	_logo_top = cfg_disp_pri_resol_height/2 + 180;
	/* end add */
	
	int s = 2;
	int l = _logo_left, t = _logo_top;
	int w = (_logo_width*s), h = (_logo_height*s);
	unsigned bg = LOGO_BGCOLOR;

	lcd_fill_rectangle(l, t, w, h, bg, 0);
	lcd_draw_string(l, t, s, s, 0, "%s: %s", part, stat);
}

void fboot_lcd_status(char *stat)
{
	/* add by cym 20151127 */
	_logo_left = cfg_disp_pri_resol_width /2 +  50;
	_logo_top = cfg_disp_pri_resol_height/2 + 180;
	/* end add */
	
	int s = 2;
	int l = _logo_left, t = _logo_top;
	int w = (_logo_width*s), h = (_logo_height*s);
	unsigned bg = LOGO_BGCOLOR;

	lcd_fill_rectangle(l, t, w, h, bg, 0);
	lcd_draw_string(l, t, s, s, 0, "%s", stat);
}
