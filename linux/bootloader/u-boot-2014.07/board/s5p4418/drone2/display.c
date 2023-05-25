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
#include <asm/arch/mach-api.h>
#include <asm/arch/display.h>

/* add by cym 20151127 */
#include <cfg_common.h>
/* end add */

/* add by cym 20150811 */
extern void display_lvds(int module, unsigned int fbbase,
                                        struct disp_vsync_info *pvsync, struct disp_syncgen_param *psgen,
                                        struct disp_multily_param *pmly, struct disp_lvds_param *plvds);
/* end add */

extern void display_rgb(int module, unsigned int fbbase,
					struct disp_vsync_info *pvsync, struct disp_syncgen_param *psgen,
					struct disp_multily_param *pmly, struct disp_rgb_param *prgb);

#define	INIT_VIDEO_SYNC(name)								\
	struct disp_vsync_info name = {							\
		.h_active_len	= cfg_disp_pri_resol_width,         \
		.h_sync_width	= cfg_disp_pri_hsync_sync_width,    \
		.h_back_porch	= cfg_disp_pri_hsync_back_porch,    \
		.h_front_porch	= cfg_disp_pri_hsync_front_porch,   \
		.h_sync_invert	= cfg_disp_pri_hsync_active_high,   \
		.v_active_len	= cfg_disp_pri_resol_height,        \
		.v_sync_width	= cfg_disp_pri_vsync_sync_width,    \
		.v_back_porch	= cfg_disp_pri_vsync_back_porch,    \
		.v_front_porch	= cfg_disp_pri_vsync_front_porch,   \
		.v_sync_invert	= cfg_disp_pri_vsync_active_high,   \
		.pixel_clock_hz	= cfg_disp_pri_pixel_clock,   		\
		.clk_src_lv0	= cfg_disp_pri_clkgen0_source,      \
		.clk_div_lv0	= cfg_disp_pri_clkgen0_div,         \
		.clk_src_lv1	= cfg_disp_pri_clkgen1_source,      \
		.clk_div_lv1	= cfg_disp_pri_clkgen1_div,         \
	};

#define	INIT_PARAM_SYNCGEN(name)						\
	struct disp_syncgen_param name = {						\
		.interlace 		= cfg_disp_pri_mlc_interlace,       \
		.out_format		= cfg_disp_pri_out_format,          \
		.lcd_mpu_type 	= 0,                                \
		.invert_field 	= cfg_disp_pri_out_invert_field,    \
		.swap_RB		= cfg_disp_pri_out_swaprb,          \
		.yc_order		= cfg_disp_pri_out_ycorder,         \
		.delay_mask		= 0,                                \
		.vclk_select	= cfg_disp_pri_padclksel,           \
		.clk_delay_lv0	= cfg_disp_pri_clkgen0_delay,       \
		.clk_inv_lv0	= cfg_disp_pri_clkgen0_invert,      \
		.clk_delay_lv1	= cfg_disp_pri_clkgen1_delay,       \
		.clk_inv_lv1	= cfg_disp_pri_clkgen1_invert,      \
		.clk_sel_div1	= cfg_disp_pri_clksel1_select,		\
	};

#define	INIT_PARAM_MULTILY(name)					\
	struct disp_multily_param name = {						\
		.x_resol		= cfg_disp_pri_resol_width,			\
		.y_resol		= cfg_disp_pri_resol_height,		\
		.pixel_byte		= cfg_disp_pri_screen_pixel_byte,	\
		.fb_layer		= cfg_disp_pri_screen_layer,		\
		.video_prior	= cfg_disp_pri_video_priority,		\
		.mem_lock_size	= 16,								\
		.rgb_format		= cfg_disp_pri_screen_rgb_format,	\
		.bg_color		= cfg_disp_pri_back_ground_color,	\
		.interlace		= cfg_disp_pri_mlc_interlace,		\
	};

#define	INIT_PARAM_RGB(name)							\
	struct disp_rgb_param name = {							\
		.lcd_mpu_type 	= 0,                                \
	};

/* add by cym 20150811 */
#define INIT_PARAM_LVDS(name)                                                   \
        struct disp_lvds_param name = {                                                 \
                .lcd_format     = cfg_disp_lvds_lcd_format,         \
        };
/* end add */

int bd_display(void)
{
#if defined(CONFIG_DISPLAY_OUT_RGB)
	INIT_VIDEO_SYNC(vsync);
	INIT_PARAM_SYNCGEN(syncgen);
	INIT_PARAM_MULTILY(multily);
	INIT_PARAM_RGB(rgb);

	display_rgb(cfg_disp_output_modole, CONFIG_FB_ADDR,
		&vsync, &syncgen, &multily, &rgb);
#endif

/* add by cym 20150811 */
#if defined(CONFIG_DISPLAY_OUT_LVDS)
#ifndef CONFIG_DISPLAY_OUT_RGB
        INIT_VIDEO_SYNC(vsync);
        INIT_PARAM_SYNCGEN(syncgen);
        INIT_PARAM_MULTILY(multily);
#endif
        INIT_PARAM_LVDS(lvds);

        display_lvds(cfg_disp_output_modole, CONFIG_FB_ADDR,
                &vsync, &syncgen, &multily, &lvds);
#endif
/* end add */

	return 0;
}
