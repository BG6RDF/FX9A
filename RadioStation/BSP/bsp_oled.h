#ifndef __BSP_OLED_H__
#define __BSP_OLED_H__

#include "bsp_board.h"

void spi_io_init();
void spi_write_command(uint8_t cmd);
void spi_write_data(uint8_t data);

void Set_Start_Column(unsigned char d);
void Set_Addressing_Mode(unsigned char d);
void Set_Column_Address(unsigned char a, unsigned char b);
void Set_Page_Address(unsigned char a, unsigned char b);
void Set_Start_Line(unsigned char d);
void Set_Contrast_Control(unsigned char d);
void Set_Area_Brightness(unsigned char d);
void Set_Segment_Remap(unsigned char d);
void Set_Entire_Display(unsigned char d);
void Set_Inverse_Display(unsigned char d);
void Set_Multiplex_Ratio(unsigned char d);
void Set_Dim_Mode(unsigned char a, unsigned char b);
void Set_Master_Config(unsigned char d);
void Set_Display_On_Off(unsigned char d);
void Set_Start_Page(unsigned char d);
void Set_Common_Remap(unsigned char d);
void Set_Display_Offset(unsigned char d);
void Set_Display_Clock(unsigned char d);
void Set_Area_Color(unsigned char d);
void Set_Precharge_Period(unsigned char d);
void Set_Common_Config(unsigned char d);
void Set_VCOMH(unsigned char d);
void Set_Read_Modify_Write(unsigned char d);
void Set_NOP();

#endif