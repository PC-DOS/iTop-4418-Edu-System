# iTop-4418内核及uBoot源代码修改日志：显示器配置

本修改用于支持使用640x480分辨率的显示器。

## uBoot修改

修改`u-boot/board/s5p4418/drone2/board.c`中的以下代码，即为默认显示参数：

```c++
int cfg_disp_output_modole = 0;

int cfg_disp_pri_screen_layer = 0;
int cfg_disp_pri_screen_rgb_format = MLC_RGBFMT_A8R8G8B8;
int cfg_disp_pri_screen_pixel_byte = 4;
int cfg_disp_pri_screen_color_key = 0x090909;

int cfg_disp_pri_video_priority = 2;	// 0, 1, 2, 3;
int cfg_disp_pri_back_ground_color = 0x000000;

int cfg_disp_pri_mlc_interlace = CFALSE;

int cfg_disp_pri_resol_width = 640;
int cfg_disp_pri_resol_height = 480;

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
```

同时，将该文件内`p = getenv("bootargs");`语句后的`if`语句的判断条件改为`if (1)`。

在该文件的`board_late_init()`函数中，有一个`if(!strcmp(p, "9.7")){ ... }`结构体，这个结构体用于根据`lcdtype`环境变量的值配置屏幕参数。这里添加一个`640x480`值，并修改`else`分支的设置：

```c++
if(!strcmp(p, "9.7")){ 
    //... 
}
//...
//Modified by Picsell Dois, for support of 640x480 screen
else if(!strcmp(p, "640x480"))
        {
                cfg_disp_pri_resol_width = 640;
                cfg_disp_pri_resol_height= 480;
        }
else
{
    //Modified by Picsell Dois, for support of 640x480 screen
    cfg_disp_pri_resol_width = 640;
    cfg_disp_pri_resol_height = 480;
}
```

修改`u-boot/include/configs/s5p4418_drone2.h`文件中`lcdtype`环境变量的默认配置参数定义`CONFIG_DISPLAY_LCD_TYPE`为`640x480`值。

```
#define CONFIG_DISPLAY_LCD_TYPE "640x480"
```

## Linux内核修改

修改`kernel/arch/arm/plat-s5p4418/topeet/include/cfg_main.h`中的默认显示参数：

```c++
/*------------------------------------------------------------------------------
 * 	Display (DPC and MLC)
 */
/* Primary */
#define CFG_DISP_PRI_SCREEN_LAYER               0
#define CFG_DISP_PRI_SCREEN_RGB_FORMAT          MLC_RGBFMT_X8R8G8B8//MLC_RGBFMT_A8R8G8B8
#define CFG_DISP_PRI_SCREEN_PIXEL_BYTE	        4
#define CFG_DISP_PRI_SCREEN_COLOR_KEY	        0x090909

#define CFG_DISP_PRI_VIDEO_PRIORITY				2	// 0, 1, 2, 3
#define CFG_DISP_PRI_BACK_GROUND_COLOR	     	0x000000

#define CFG_DISP_PRI_MLC_INTERLACE              CFALSE

#define	CFG_DISP_PRI_LCD_WIDTH_MM				154
#define	CFG_DISP_PRI_LCD_HEIGHT_MM				85

/* modify by cym 20150811 */
#if 0
#define CFG_DISP_PRI_RESOL_WIDTH          		640	// X Resolution
#define CFG_DISP_PRI_RESOL_HEIGHT				480	// Y Resolution
#else
#define CFG_DISP_PRI_RESOL_WIDTH                        640//1024    // X Resolution
#define CFG_DISP_PRI_RESOL_HEIGHT                               480//768     // Y Resolution
#endif
/* end remove */

#define CFG_DISP_PRI_HSYNC_SYNC_WIDTH            20
#define CFG_DISP_PRI_HSYNC_BACK_PORCH           160
#define CFG_DISP_PRI_HSYNC_FRONT_PORCH          160
#define CFG_DISP_PRI_HSYNC_ACTIVE_HIGH          CTRUE
#define CFG_DISP_PRI_VSYNC_SYNC_WIDTH            3
#define CFG_DISP_PRI_VSYNC_BACK_PORCH            23
#define CFG_DISP_PRI_VSYNC_FRONT_PORCH           12
#define CFG_DISP_PRI_VSYNC_ACTIVE_HIGH 	        CTRUE

#define CFG_DISP_PRI_CLKGEN0_SOURCE             DPC_VCLK_SRC_PLL2
#define CFG_DISP_PRI_CLKGEN0_DIV                12 // even divide
#define CFG_DISP_PRI_CLKGEN0_DELAY              0
#define CFG_DISP_PRI_CLKGEN0_INVERT				0
#define CFG_DISP_PRI_CLKGEN1_SOURCE             DPC_VCLK_SRC_VCLK2
#define CFG_DISP_PRI_CLKGEN1_DIV                1
#define CFG_DISP_PRI_CLKGEN1_DELAY              0
#define CFG_DISP_PRI_CLKGEN1_INVERT				0
#define CFG_DISP_PRI_CLKSEL1_SELECT				0
#define CFG_DISP_PRI_PADCLKSEL                  DPC_PADCLKSEL_VCLK	/* VCLK=CLKGEN1, VCLK12=CLKGEN0 */

#define	CFG_DISP_PRI_PIXEL_CLOCK				800000000/CFG_DISP_PRI_CLKGEN0_DIV

#define	CFG_DISP_PRI_OUT_SWAPRB 				CFALSE
#define CFG_DISP_PRI_OUT_FORMAT                 DPC_FORMAT_RGB888
#define CFG_DISP_PRI_OUT_YCORDER                DPC_YCORDER_CbYCrY
#define CFG_DISP_PRI_OUT_INTERLACE              CFALSE
#define CFG_DISP_PRI_OUT_INVERT_FIELD           CFALSE
#define CFG_DISP_LCD_MPY_TYPE						0
```

同时修改`kernel/arch/arm/plat-s5p4418/topeet/device.c`中的`setup_width_height_param()`函数，在`if (!strncasecmp("9.7", str, 3)) { ... }`中添加对uBoot`lcdtype`环境变量`640x480`值的支持：

```c++
/* Added by Picsell Dois for support of 640x480 screen */
#if 1
	else if(!strncasecmp("640x480", str, 7))
	{
#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
                //printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
                fb0_plat_data.x_resol = 640;
                fb0_plat_data.y_resol = 480;
#endif
#endif
	}
#endif
/* end add */
```

## Qt/E环境变量修改

修改Qt/E根文间系统中对`QWS_DISPLAY`变量的导出，避免字体过大问题，将`/bin/qt4`中的`export QWS_DISPLAY=Transformed:Rot0`行改为：

```
export QWS_DISPLAY=LinuxFb:mmWidth240:mmHeight180:1
```
