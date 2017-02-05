#ifndef __BSP_OLED_API_H__
#define __BSP_OLED_API_H__

#include "bsp_oled.h"

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Global Variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define	Shift		0x1C

#define Max_Column	0x3F			// 256/4-1
#define Max_Row		0x3F			// 64-1
#define	Brightness	0xff

void oled_init();

extern unsigned char ASCII_1608[72][16];
//ÏÔÊ¾ÃüÁîº¯Êý
void Fill_RAM(unsigned char Data);
void Fill_Block(unsigned char Data, unsigned char a, unsigned char b, unsigned char c, unsigned char d);
void Checkerboard();
void Frame();
 
void Show_Font57(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
 
void Show_String(unsigned char a, unsigned char *Data_Pointer, unsigned char b, unsigned char c);
 
void Show_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d);
//void Continuous_Scroll(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g, unsigned char h, unsigned char i);
void Deactivate_Scroll();
void Fade_In();
void Fade_Out();
void Sleep(unsigned char a);
void Test();
//void Set_LUT(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
//void Set_Bank_Color();
 //1325
void Set_Command_Lock(unsigned char d); 
void Set_GPIO(unsigned char d);
void Set_Function_Selection(unsigned char d);
void Set_Display_Enhancement_A(unsigned char a, unsigned char b);
void Set_Partial_Display(unsigned char a, unsigned char b, unsigned char c);
void Set_Write_RAM();
void Set_Master_Current(unsigned char d);
void Set_Display_Enhancement_B(unsigned char d);
void Set_Remap_Format(unsigned char d);
void Set_Current_Range(unsigned char d);
void Set_Gray_Scale_Table();
void Set_Contrast_Current(unsigned char d);
void Set_Frame_Frequency(unsigned char d);
void Set_Phase_Length(unsigned char d);
void Set_Precharge_Voltage(unsigned char d);
void Set_Precharge_Compensation(unsigned char a, unsigned char b);
void Set_VSL(unsigned char d);
void Set_Display_Mode(unsigned char d);
void Set_Row_Address(unsigned char a, unsigned char b);
void Vertical_Scroll(unsigned char a, unsigned char b, unsigned char c);
void Horizontal_Scroll(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
 
void Show_TFREQ(uint8_t x, uint8_t y, uint32_t freq);
void Show_TFREQ_Clear(uint8_t x, uint8_t y);

void Fade_Scroll(unsigned char a, unsigned char b, unsigned char c);
void Grayscale();
 
//void Show_Pattern_mono(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d);
void Copy(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f);	//ÏÔ´æÇøÓò¸´ÖÆ

void con_4_byte(uint8_t DATA);								 //×Ö½Ú×ª»»³ÌÐò
void hz_1616(uint8_t x,uint8_t y,uint8_t *Data_Pointer);	//16*16ºº×Ö
void oled_ascii_3216(uint8_t x, uint8_t y, uint8_t *data);
void Show_dot(unsigned char x, unsigned char y);		   //»­µã
void ascii_1608(uint8_t x,uint8_t y,uint8_t a[16]);	  //16*8ascii
void Draw_Rectangle(unsigned char Data, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e);
void oled_draw_scale(uint8_t x, uint8_t y);
void Show_Font0507(unsigned char b, unsigned char c, unsigned char d);
void Show_CALL_TYPE(uint8_t x, uint8_t y, uint8_t type);
void oled_update_mem(uint8_t val);

void oled_update_vol(uint8_t val);

void oled_update_att(uint8_t val);

void oled_update_bfo(uint16_t val);
void oled_h_vswr_disp();
void oled_h_vswr_clear();

void oled_update_wpm(uint8_t val);
void oled_close_wpm();
void oled_ascii_3216_dot(uint8_t x, uint8_t y, uint8_t *data);
void oled_update_dc(uint16_t val);

void oled_lsb_usb(uint8_t *str);
void oled_lsb_usb_mem(uint8_t *str);

void oled_ts(uint8_t *str);
void oled_rit(uint8_t *str);
void oled_tx_power(uint8_t *str);

void oled_filter(uint8_t *str);
void oled_txrx(uint8_t *str);

void oled_rx_grid(uint8_t num);
void oled_swr_grid(uint8_t num);
void oled_rf_grid(uint8_t num);
void oled_txrx_clear_grid();
void oled_update_frequency(uint32_t freq, uint8_t all);
void oled_update_freq(uint8_t three, uint16_t two, uint8_t one, uint8_t all);

void oled_vfo(uint8_t *str);
void oled_set_screen(uint8_t Data);
void oled_update_rx_scaler();
void oled_update_tx_scaler();

void oled_ascii_1608(uint8_t x, uint8_t y, uint8_t *str);
void oled_mem_set(uint8_t position, uint32_t freq, uint8_t all);
void oled_mem_arrows_set(uint8_t position);
void oled_menu_set(uint8_t position);
void oled_menu_arrows_set(uint8_t position);

void oled_rit_txfreq( uint32_t freq);
void oled_rit_txfreq_clear();
void oled_display_on();
void oled_display_off();
void oled_button_lock(uint8_t val);
#endif