#include "bsp_oled_api.h"
#include "zk.h"
#include "bsp_pwm.h"
#include "bsp_transmit_power.h"

//unsigned char Ascii_1[240][5];
//unsigned char Ascii_2[107][5];

uint8_t arrows_pos;
uint8_t menu_pos;

uint8_t freq_three = 0xff;
uint16_t freq_two = 0xffff;
uint8_t freq_one = 0xff;

uint8_t mem_set_three = 0xff;
uint16_t mem_set_two = 0xffff;
uint8_t mem_set_one = 0xff;

bool dot = false;

void oled_init_disp();
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Graphic Acceleration
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void oled_init()
{
  spi_io_init();
  Set_Display_On_Off(0x00);	
  Set_Command_Lock(0x12);			// Unlock Basic Commands (0x12/0x16)
  Set_Display_On_Off(0x00);		// Display Off (0x00/0x01)
  Set_Display_Clock(0x91);		// Set Clock as 80 Frames/Sec
  Set_Multiplex_Ratio(0x3F);		// 1/64 Duty (0x0F~0x3F)
  Set_Display_Offset(0x00);		// Shift Mapping RAM Counter (0x00~0x3F)
  Set_Start_Line(0x00);			// Set Mapping RAM Display Start Line (0x00~0x7F)
  Set_Remap_Format(0x14);			// Set Horizontal Address Increment
  //     Column Address 0 Mapped to SEG0
  //     Disable Nibble Remap
  //     Scan from COM[N-1] to COM0
  //     Disable COM Split Odd Even
  //     Enable Dual COM Line Mode
  Set_GPIO(0x00);				// Disable GPIO Pins Input
  Set_Function_Selection(0x01);		// Enable Internal VDD Regulator
  Set_Display_Enhancement_A(0xA0,0xFD);	// Enable External VSL
  // Set Low Gray Scale Enhancement
  Set_Contrast_Current(0xEF);		// Set Segment Output Current
  Set_Master_Current(Brightness);		// Set Scale Factor of Segment Output Current Control
  Set_Gray_Scale_Table();			// Set Pulse Width for Gray Scale Table
  Set_Phase_Length(0xE2);			// Set Phase 1 as 5 Clocks & Phase 2 as 14 Clocks
  Set_Display_Enhancement_B(0x20);	// Enhance Driving Scheme Capability (0x00/0x20)
  Set_Precharge_Voltage(0x1F);		// Set Pre-Charge Voltage Level as 0.60*VCC
  Set_Precharge_Period(0x08);		// Set Second Pre-Charge Period as 8 Clocks
  Set_VCOMH(0x07);			// Set Common Pins Deselect Voltage Level as 0.86*VCC
  Set_Display_Mode(0x02);			// Normal Display Mode (0x00/0x01/0x02/0x03)
  Set_Partial_Display(0x01,0x00,0x00);	// Disable Partial Display
  
  //Fill_RAM(0x00);				// Clear Screen
  oled_set_screen(0x00);
  
  
  oled_init_disp();
  Set_Display_On_Off(0x00);		// Display On (0x00/0x01)
//  oled_rf_grid(1);
//  oled_rf_grid(2);
//  oled_rf_grid(6);
//  oled_rf_grid(4);
}

void oled_display_on()
{
  Set_Display_On_Off(0x01);		// Display On (0x00/0x01)
}

void oled_display_off()
{
  Set_Display_On_Off(0x00);		// Display On (0x00/0x01)
}
 //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Graphic Acceleration (Partial or Full Screen)
//
//    a: Line Width
//    b: Column Address of Start
//    c: Column Address of End
//    d: Row Address of Start
//    e: Row Address of End
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Draw_Rectangle(unsigned char Data, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e)
{
  unsigned char i,j,k,l;
  
  k=a%4;
  if(k == 0)
  {
    l=(a/4)-1;  //计算行数
  }
  else
  {
    l=a/4;
  }
  
  Set_Column_Address(Shift+b,Shift+c);  // 
  Set_Row_Address(d,(d+a-1));
  Set_Write_RAM();
  for(i=0;i<(c-b+1);i++)
  {
    for(j=0;j<a;j++)
    {
      spi_write_data(Data);
      spi_write_data(Data);
    }
  }
  
  Set_Column_Address(Shift+(c-l),Shift+c);
  Set_Row_Address(d+a,e-a);
  Set_Write_RAM();
  for(i=0;i<(e-d+1);i++)
  {
    for(j=0;j<(l+1);j++)
    {
      if(j == 0)
      {
        switch(k)
        {
        case 0:
          spi_write_data(Data);
          spi_write_data(Data);
          break;
        case 1:
          spi_write_data(0x00);
          spi_write_data(Data&0x0F);
          break;
        case 2:
          spi_write_data(0x00);
          spi_write_data(Data);
          break;
        case 3:
          spi_write_data(Data&0x0F);
          spi_write_data(Data);
          break;
        }
      }
      else
      {
        spi_write_data(Data);
        spi_write_data(Data);
      }
    }
  }
  
  Set_Column_Address(Shift+b,Shift+c);
  Set_Row_Address((e-a+1),e);
  Set_Write_RAM();
  for(i=0;i<(c-b+1);i++)
  {
    for(j=0;j<a;j++)
    {
      spi_write_data(Data);
      spi_write_data(Data);
    }
  }
  
  Set_Column_Address(Shift+b,Shift+(b+l));
  Set_Row_Address(d+a,e-a);
  Set_Write_RAM();
  for(i=0;i<(e-d+1);i++)
  {
    for(j=0;j<(l+1);j++)
    {
      if(j == l)
      {
        switch(k)
        {
        case 0:
          spi_write_data(Data);
          spi_write_data(Data);
          break;
        case 1:
          spi_write_data(Data&0xF0);
          spi_write_data(0x00);
          break;
        case 2:
          spi_write_data(Data);
          spi_write_data(0x00);
          break;
        case 3:
          spi_write_data(Data);
          spi_write_data(Data&0xF0);
          break;
        }
      }
      else
      {
        spi_write_data(Data);
        spi_write_data(Data);
      }
    }
  }
}

void oled_set_screen(uint8_t Data)
{
  unsigned char i,j;
  
  Set_Column_Address(0x00,0x77);
  Set_Row_Address(0x00,0x7F);
  Set_Write_RAM();
  
  for(i=0;i<128;i++)
  {
    for(j=0;j<64;j++)
    {
      spi_write_data(Data);
      spi_write_data(Data);
    }
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_RAM(unsigned char Data)
{
  unsigned char i,j;
  
  Set_Column_Address(0x00,0x77);
  Set_Row_Address(0x00,0x7F);
  Set_Write_RAM();
  
  for(i=0;i<128;i++)
  {
    for(j=0;j<120;j++)
    {
      spi_write_data(Data);
      spi_write_data(Data);
    }
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End (Total Columns Devided by 4)
//    c: Row Address of Start
//    d: Row Address of End
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_Block(unsigned char Data, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
  unsigned char i,j;
  
  Set_Column_Address(Shift+a,Shift+b);
  Set_Row_Address(c,d);
  Set_Write_RAM();
  
  for(i=0;i<(d-c+1);i++)
  {
    for(j=0;j<(b-a+1);j++)
    {
      spi_write_data(Data);
      spi_write_data(Data);
    }
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Checkboard (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Checkerboard()
{
  unsigned char i,j;
  
  Set_Column_Address(0x00,0x77);
  Set_Row_Address(0x00,0x7F);
  Set_Write_RAM();
  
  for(i=0;i<64;i++)
  {
    for(j=0;j<120;j++)
    {
      spi_write_data(0xF0);
      spi_write_data(0xF0);
    }
    for(j=0;j<120;j++)
    {
      spi_write_data(0x0F);
      spi_write_data(0x0F);
    }
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Gray Scale Bar (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Grayscale()
{
  // Level 16 => Column 1~16
  Fill_Block(0xFF,0x00,0x03,0x00,Max_Row);
  
  // Level 15 => Column 17~32
  Fill_Block(0xEE,0x04,0x07,0x00,Max_Row);
  
  // Level 14 => Column 33~48
  Fill_Block(0xDD,0x08,0x0B,0x00,Max_Row);
  
  // Level 13 => Column 49~64
  Fill_Block(0xCC,0x0C,0x0F,0x00,Max_Row);
  
  // Level 12 => Column 65~80
  Fill_Block(0xBB,0x10,0x13,0x00,Max_Row);
  
  // Level 11 => Column 81~96
  Fill_Block(0xAA,0x14,0x17,0x00,Max_Row);
  
  // Level 10 => Column 97~112
  Fill_Block(0x99,0x18,0x1B,0x00,Max_Row);
  
  // Level 9 => Column 113~128
  Fill_Block(0x88,0x1C,0x1F,0x00,Max_Row);
  
  // Level 8 => Column 129~144
  Fill_Block(0x77,0x20,0x23,0x00,Max_Row);
  
  // Level 7 => Column 145~160
  Fill_Block(0x66,0x24,0x27,0x00,Max_Row);
  
  // Level 6 => Column 161~176
  Fill_Block(0x55,0x28,0x2B,0x00,Max_Row);
  
  // Level 5 => Column 177~192
  Fill_Block(0x44,0x2C,0x2F,0x00,Max_Row);
  
  // Level 4 => Column 193~208
  Fill_Block(0x33,0x30,0x33,0x00,Max_Row);
  
  // Level 3 => Column 209~224
  Fill_Block(0x22,0x34,0x37,0x00,Max_Row);
  
  // Level 2 => Column 225~240
  Fill_Block(0x11,0x38,0x3B,0x00,Max_Row);
  
  // Level 1 => Column 241~256
  Fill_Block(0x00,0x3C,Max_Column,0x00,Max_Row);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show String0705: 显示字符串
//      BG6RDF
//    x: Start X Address
//    y: Start Y Address
//    str: 字符串指针
//    
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_String0705(uint8_t x, uint8_t y, uint8_t *str)
{
  //OLED分辨率256*64，因此0705的字符外加每个字符间一个像素的空隙，一行最多显示43个字符
  uint8_t row[7][43];
  uint8_t len=strlen((char const *)str);        //字符串长度
  uint8_t tlen=len*6-1;                         //每行总像素数量
  uint8_t bytelen=(tlen+7)/8;                   //每行目标字节数
  tlen=bytelen*8/4-1;
  
  for(int i=0; i<7; i++)        //7行像素
  {
    int startBit=8;             //复制到目标字节的开始位数，最高位是8，最低位是1
    int processedBit=0;         //因一个字符的一行可能跨多个目标字节，因此要记录已处理了多少位
    int byteIdx=0;              //目标字节的下标
    
    row[i][0]=0;
    for(int j=0; j<len; )
    {
      //先左移动3位，对齐最高位，如果已处理了几位再加上processedBit，再减去起始位相对最高位的偏移
      int shiftBit=3+processedBit-(8-startBit);
      int firstBit=5-processedBit;
      //字符在当前目标字节的有效位数
      int effectiveBit=startBit>=firstBit ? firstBit : startBit;
      uint8_t bits=ASCII_0705[str[j]-0x20][i];
      if (shiftBit>0)
        bits<<=shiftBit;
      else if (shiftBit<0)
        bits>>=(-shiftBit);
      //把有效位数以外的所有位置为0
      //掩码先生成一个有效位数长度的1位，然后移动到起始位
      uint8_t maskBit=((1<<effectiveBit)-1)<<(startBit-effectiveBit);
      bits&=maskBit;
      //设置目标字节
      row[i][byteIdx] |= bits ;
      
      //移动目标位指针
      startBit-=effectiveBit;
      
      //当前字符是否已处理完
      if (processedBit+effectiveBit>=5)
      {
        j++;                    //当前字符已处理完
        processedBit=0;
        if (startBit>0)         //目标字节还有空位
          startBit--;           //相当于插入一个空像素
        else
        {
          startBit=7;           //相当于插入一个空像素
          row[i][++byteIdx]=0;  //下一个目标字节清空
        }
        //if (':'==str[j] || '.'==str[j])
        //  processedBit=3;       //冒号和句号只占用两个像素宽
      }
      else
        processedBit+=effectiveBit;     //当前字符还有余下的位

      if (startBit<=0)    //插入空像素后没空位了
      {
        startBit=8;
        row[i][++byteIdx]=0;    //下一个目标字节清空
      }
    }
  }

  Set_Column_Address(Shift+x,Shift+x+tlen);
  Set_Row_Address(y,y+7);
  Set_Write_RAM();
  
  for(uint8_t i=0; i<7; i++)
  {
    for(uint8_t j=0; j<bytelen; j++)
    {
      con_4_byte(row[i][j]);  
    }
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show T 14.270.00
//
//    x: Start X Address
//    y: Start Y Address
//    freq: 
//    
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_TFREQ(uint8_t x, uint8_t y, uint32_t freq)
{
  uint8_t strbuf[12];
  if (FREQ.item & RITItem)
    strbuf[0]='T';
  else
    strbuf[0]='R';
  strbuf[1]=' ';
  
  uint8_t chr=freq%10000000/1000000;
  if (0==chr)
    chr=' ';
  else
    chr+='0';
  strbuf[2]=chr;
  strbuf[3]=freq%1000000/100000+'0';
  strbuf[4]='.';
  strbuf[5]=freq%100000/10000+'0';
  strbuf[6]=freq%10000/1000+'0';
  strbuf[7]=freq%1000/100+'0';
  strbuf[8]='.';
  strbuf[9]=freq%100/10+'0';
  strbuf[10]=freq%10+'0';
  strbuf[11]=0;
  
  Show_String0705(x, y, strbuf);
}

void Show_TFREQ_Clear(uint8_t x, uint8_t y)
{
  uint8_t strbuf[12]="           ";
  Show_String0705(x, y, strbuf);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show DC:val
//
//    x: Start X Address
//    y: Start Y Address
//    val: 0.00~Vcc
//    
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_DC(uint8_t x, uint8_t y, uint16_t val)
{
  uint8_t strbuf[9]="DC:";
  strbuf[3]=val%1000/100+'0';
  strbuf[4]=val%100/10+'0';
  strbuf[5]='.';
  strbuf[6]=val%10+'0';
  strbuf[7]='V';
  strbuf[8]=0;
  
  Show_String0705(x, y, strbuf);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show WPM:val
//
//    x: Start X Address
//    y: Start Y Address
//    val: 8~50
//    
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_WPM(uint8_t x, uint8_t y, uint8_t val)
{
  uint8_t strbuf[7]="WPM:";
  strbuf[4]=val%100/10+'0';
  strbuf[5]=val%10+'0';
  strbuf[6]=0;
  
  Show_String0705(x, y, strbuf);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show VOL:60
//
//    x: Start X Address
//    y: Start Y Address
//    val: 0~60
//    
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void Show_VOL(uint8_t x, uint8_t y, uint8_t val)
{
  uint8_t strbuf[7]="VOL:";
  strbuf[4]=val%100/10+'0';
  strbuf[5]=val%10+'0';
  strbuf[6]=0;
  Show_String0705(x, y, strbuf);
}

void Show_CH(uint8_t x, uint8_t y, uint8_t val)
{
  uint8_t strbuf[6]="CH:";
  strbuf[3]=val%100/10+'0';
  strbuf[4]=val%10+'0';
  strbuf[5]=0;
  Show_String0705(x, y, strbuf);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Character (5x7)
//
//    a: Database
//    b: Ascii
//    c: Start X Address
//    d: Start Y Address
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_Font57(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
  unsigned char *Src_Pointer;
  unsigned char i,Font,MSB_1,LSB_1,MSB_2,LSB_2;
  
  switch(a)
  {
  case 1:
    Src_Pointer=&Ascii_1[(b-1)][0];
    break;
  case 2:
    Src_Pointer=&Ascii_2[(b-1)][0];
    break;
  }
  
  Set_Remap_Format(0x15);
  for(i=0;i<=1;i++)
  {
    MSB_1=*Src_Pointer;
    Src_Pointer++;
    if(i == 1)
    {
      LSB_1=0x00;
      MSB_2=0x00;
      LSB_2=0x00;
    }
    else
    {
      LSB_1=*Src_Pointer;
      Src_Pointer++;
      MSB_2=*Src_Pointer;
      Src_Pointer++;
      LSB_2=*Src_Pointer;
      Src_Pointer++;
    }
    Set_Column_Address(Shift+c,Shift+c);
    Set_Row_Address(d,d+7);
    Set_Write_RAM();
    
    Font=((MSB_1&0x01)<<4)|(LSB_1&0x01);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    Font=((MSB_2&0x01)<<4)|(LSB_2&0x01);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    
    Font=((MSB_1&0x02)<<3)|((LSB_1&0x02)>>1);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    Font=((MSB_2&0x02)<<3)|((LSB_2&0x02)>>1);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    
    Font=((MSB_1&0x04)<<2)|((LSB_1&0x04)>>2);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    Font=((MSB_2&0x04)<<2)|((LSB_2&0x04)>>2);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    
    Font=((MSB_1&0x08)<<1)|((LSB_1&0x08)>>3);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    Font=((MSB_2&0x08)<<1)|((LSB_2&0x08)>>3);
    Font=Font|(Font<<1)|(Font<<2)|(Font<<3);
    spi_write_data(Font);
    
    Font=((MSB_1&0x10)<<3)|((LSB_1&0x10)>>1);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    Font=((MSB_2&0x10)<<3)|((LSB_2&0x10)>>1);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    
    Font=((MSB_1&0x20)<<2)|((LSB_1&0x20)>>2);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    Font=((MSB_2&0x20)<<2)|((LSB_2&0x20)>>2);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    
    Font=((MSB_1&0x40)<<1)|((LSB_1&0x40)>>3);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    Font=((MSB_2&0x40)<<1)|((LSB_2&0x40)>>3);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    
    Font=(MSB_1&0x80)|((LSB_1&0x80)>>4);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    Font=(MSB_2&0x80)|((LSB_2&0x80)>>4);
    Font=Font|(Font>>1)|(Font>>2)|(Font>>3);
    spi_write_data(Font);
    
    c++;
  }
  Set_Remap_Format(0x14);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show String
//
//    a: Database
//    b: Start X Address
//    c: Start Y Address
//    * Must write "0" in the end...
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_String(unsigned char a, unsigned char *Data_Pointer, unsigned char b, unsigned char c)
{
unsigned char *Src_Pointer;

	Src_Pointer=Data_Pointer;
	Show_Font57(1,96,b,c);			// No-Break Space
						//   Must be written first before the string start...

	while(1)
	{
		Show_Font57(a,*Src_Pointer,b,c);
		Src_Pointer++;
		b+=2;
		if(*Src_Pointer == 0) break;
	}
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End (Total Columns Devided by 4)
//    c: Row Address of Start
//    d: Row Address of End
// 灰度模式下显示一副图片
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
			 
unsigned char *Src_Pointer;
unsigned char i,j;
//unsigned char temp1,temp2;
 /*	   取模时像素反序
Set_Remap_Format(0x10);	
	Src_Pointer=Data_Pointer;
	Set_Column_Address(Shift+a,Shift+b);
	Set_Row_Address(c,d);
	Set_Write_RAM();
 //	Src_Pointer++;
	for(i=0;i<(d-c+1);i++)
	{
		for(j=0;j<(b-a+1);j++)
		{
			
			temp1=*Src_Pointer;
			Src_Pointer++;
			temp2=*Src_Pointer;
			spi_write_data(temp2);
         	spi_write_data(temp1);
			Src_Pointer++;
	     
		}
	}
 */
 //取模时候像素正序	（不能反序与2.7不同）
    Src_Pointer=Data_Pointer;
	Set_Column_Address(Shift+a,Shift+b);
	Set_Row_Address(c,d);
	Set_Write_RAM();

	for(i=0;i<(d-c+1);i++)
	{
		for(j=0;j<(b-a+1);j++)
		{
			spi_write_data(*Src_Pointer);
			Src_Pointer++;
			spi_write_data(*Src_Pointer);
			Src_Pointer++;
		}
	}


}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Vertical Scrolling (Full Screen)
//
//    a: Scrolling Direction
//       "0x00" (Upward)
//       "0x01" (Downward)
//    b: Set Numbers of Row Scroll per Step
//    c: Set Time Interval between Each Scroll Step
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Vertical_Scroll(unsigned char a, unsigned char b, unsigned char c)
{
unsigned char i,j;	

	Set_Partial_Display(0x00,0x00,Max_Row);

	switch(a)
	{
		case 0:
			for(i=0;i<(Max_Row+1);i+=b)
			{
				Set_Display_Offset(i+1);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			break;
		case 1:
			for(i=0;i<(Max_Row+1);i+=b)
			{
				Set_Display_Offset(Max_Row-i);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			break;
	}
	Set_Partial_Display(0x01,0x00,0x00);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Vertical Fade Scrolling (Full Screen)
//
//    a: Scrolling Direction
//       "0x00" (Upward - In)
//       "0x01" (Downward - In)
//       "0x02" (Upward - Out)
//       "0x03" (Downward - Out)
//    b: Set Numbers of Row Scroll per Step
//    c: Set Time Interval between Each Scroll Step
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_Scroll(unsigned char a, unsigned char b, unsigned char c)
{
unsigned char i,j;	

	Set_Partial_Display(0x00,0x00,Max_Row);

	switch(a)
	{
		case 0:
			for(i=(Max_Row+1);i<128;i+=b)
			{
				Set_Start_Line(i);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			Set_Start_Line(0x00);
			for(j=0;j<c;j++)
			{
				delay_us(200);
			}
			break;
		case 1:
			for(i=0;i<(Max_Row+1);i+=b)
			{
				Set_Start_Line(Max_Row-i);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			break;
		case 2:
			for(i=0;i<(Max_Row+1);i+=b)
			{
				Set_Start_Line(i+1);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			break;
		case 3:
			for(i=127;i>Max_Row;i-=b)
			{
				Set_Start_Line(i);
				for(j=0;j<c;j++)
				{
					delay_us(200);
				}
			}
			break;
	}
	Set_Partial_Display(0x01,0x00,0x00);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade In (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_In()
{
unsigned int i;	

	Set_Display_On_Off(0x01);
	for(i=0; i<(Brightness+1);i++)
	{
		Set_Master_Current(i);
		delay_us(200);
		delay_us(200);
		delay_us(200);
	}
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade Out (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_Out()
{
unsigned int i;	

	for(i=(Brightness+1);i>0;i--)
	{
		Set_Master_Current(i-1);
		delay_us(200);
		delay_us(200);
		delay_us(200);
	}
	Set_Display_On_Off(0x00);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Sleep Mode
//
//    "0x01" Enter Sleep Mode
//    "0x00" Exit Sleep Mode
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Sleep(unsigned char a)
{
	switch(a)
	{
		case 0:
			Set_Display_On_Off(0x00);
			Set_Display_Mode(0x01);
			break;
		case 1:
			Set_Display_Mode(0x02);
			Set_Display_On_Off(0x01);
			break;
	}
}

void oled_draw_scale1(uint8_t x, uint8_t y, uint8_t *data)
{
  Set_Column_Address(Shift+x,Shift+x+15); // 设置列坐标，shift为列偏移量由1322决定。15为64/4-1
  Set_Row_Address(y,y+11);  //
  Set_Write_RAM();	 //	写显存
  
  for(uint16_t j=0;j<88;j++)
  {
      con_4_byte(data[j]);	 //数据转换
  }
}

void oled_update_freq(uint8_t three, uint16_t two, uint8_t one, uint8_t all)
{
  uint8_t pos = 36;
  uint8_t pos_y = 3;
  if(freq_three != three || all)
  {
    
    if( three > 9)
    {
      oled_ascii_3216(pos, pos_y, DIGIT_3216[three%100/10]);  // 16/4-1
    }else if(freq_three > 9 || freq_three != 0xff)
    {
      oled_ascii_3216(pos, pos_y, DIGIT_3216[11]);  // 清除位
    }
    freq_three = three;
    pos += 4;
    oled_ascii_3216_dot(pos, pos_y, DIGIT_3216[three%10]);
    pos += 4;
  }else{
    pos += 8;
  }
  
  if(freq_two != two || all)
  {
    freq_two = two;
    oled_ascii_3216(pos, pos_y, DIGIT_3216[two%1000/100]);
    pos += 4;
    oled_ascii_3216(pos, pos_y, DIGIT_3216[two%100/10]);
    pos += 4;
    oled_ascii_3216_dot(pos, pos_y, DIGIT_3216[two%10]);
    pos += 4;
    
  }else{
    pos += 12;
  }
  
  if(freq_one != one || all)
  {
    freq_one = one;
    oled_ascii_3216(pos, pos_y, DIGIT_3216[one%100/10]);
    pos += 4;
    oled_ascii_3216(pos, pos_y, DIGIT_3216[one%10]) ;
  }
}

void oled_update_frequency(uint32_t freq, uint8_t all)
{
  oled_update_freq(freq%10000000/100000, freq%100000/100 ,freq%100, all);
}

void oled_update_rx_scaler()
{
  oled_draw_scale(0, 8);
}

void oled_update_tx_scaler()
{
  oled_draw_scale1(0, 28, scaler_swr);
  oled_draw_scale1(13, 28, scaler_rf);  
}

void oled_draw_grid(uint8_t x, uint8_t y)
{
  Set_Column_Address(Shift+x,Shift+x); // 设置列坐标，shift为列偏移量由1322决定。1为8/4-1
  Set_Row_Address(y,y+6);  //
  Set_Write_RAM();	 //	写显存
  
  for(uint8_t j=0;j<6;j++)
  {
    for(uint8_t i=0; i<2; i++)
    {
      if(i == 1)
      {
        spi_write_data(0xff);
      }else
      {
        spi_write_data(0x0f);
      }
    }
    
  }  
}

void oled_clear_grid(uint8_t x, uint8_t y)
{
  Set_Column_Address(Shift+x,Shift+x); // 设置列坐标，shift为列偏移量由1322决定。1为8/4-1
  Set_Row_Address(y,y+6);  //
  Set_Write_RAM();	 //	写显存
  
  for(uint8_t j=0;j<6;j++)
  {
    for(uint8_t i=0; i<2; i++)
    {
      spi_write_data(0x00);
    }
    
  }  
}

void oled_vfo(uint8_t *str)
{
  Show_String0705(36, 35, str);
}

void oled_update_mem(uint8_t val)
{
  Show_CH(36, 35, val);
}

void oled_update_vol(uint8_t val)
{
  Show_VOL(0, 57, val);
}

void oled_update_att(uint8_t val)
{
  if( val )
  {
    Show_String0705(12, 57, "RF-ATT");
  }
  else
  { 
    Show_String0705(12, 57, "      ");
  }
}

void oled_button_lock(uint8_t val)
{
  if( val )
  {
    Show_String0705(12, 57, "LOCK");
  }
  else
  { 
    Show_String0705(12, 57, "    ");
  }
}

void oled_txrx(uint8_t *str)
{
  Show_String0705(30, 57, str);
}

void oled_update_wpm(uint8_t val)
{
  Show_WPM(42, 57, val);
}

void oled_close_wpm()
{
  Show_String0705(42, 57, "      ");  
}

void oled_update_dc(uint16_t val)
{
  Show_DC(52, 57, val);
}

void oled_lsb_usb(uint8_t *str)
{
  Show_String0705(46, 35, str);
}

void oled_ts(uint8_t *str)
{
  Show_String0705(59, 35, str);
}

void oled_rit(uint8_t *str)
{
  Show_String0705(36, 47, str);
}

void oled_rit_txfreq( uint32_t freq)
{
  Show_TFREQ(47 ,47 ,freq);
}

void oled_rit_txfreq_clear()
{
  Show_TFREQ_Clear(47 ,47);
}

void oled_tx_power(uint8_t *str)
{
  Show_String0705(24, 57, str);
}

void oled_filter(uint8_t *str)
{
  Show_String0705(54, 35, str);
}

void oled_init_disp()
{
  oled_update_rx_scaler();
  oled_update_tx_scaler();
}

uint8_t rx_grid_old=0;
uint8_t swr_grid_old=0;
uint8_t rf_grid_old=0;
void oled_txrx_clear_grid()
{
  rx_grid_old   = 0;
  swr_grid_old  = 0;
  rf_grid_old   = 0;
  
  for(uint8_t i=0; i<26; i++)
  {
    oled_clear_grid(i, 21); 
  }
}


void oled_rx_grid(uint8_t num)
{
  if( num > 25)
  {
    return;
  }
  
  if(num < rx_grid_old)       // 格子减少
  {
    for(uint8_t i=rx_grid_old; i>num; i--)
    {
      if(i > 0)
      {
        oled_clear_grid(i, 21);  
      }
    }
  }
  else if(num > rx_grid_old)  // 格子增多
  {
    for(uint8_t i=rx_grid_old; i<=num; i++)
    {
      if(i > 0)
      {
        oled_draw_grid(i, 21); 
      }
    }
  } 
  rx_grid_old = num;
}


void oled_swr_grid(uint8_t num)
{
  if( num > 9)
  {
    return;
  }
  
  if(num < swr_grid_old)       // 格子减少
  {
    for(uint8_t i=swr_grid_old; i>num; i--)
    {
      if(i > 0)
      {
        oled_clear_grid(i, 21);  
      }
    }
  }
  else if(num > rx_grid_old)  // 格子增多
  {
    for(uint8_t i=swr_grid_old; i<=num; i++)
    {
      if(i > 0)
      {
        oled_draw_grid(i, 21); 
      }
    }
  } 
  swr_grid_old = num;
}


void oled_rf_grid(uint8_t num)
{
  if( num > 13)
  {
    return;
  }
  
  if(num < rf_grid_old)       // 格子减少
  {
    for(uint8_t i=rf_grid_old; i>num; i--)
    {
      if(i > 0)
      {
        oled_clear_grid(14+i, 21);  
      }
    }
  }
  else if(num > rf_grid_old)  // 格子增多
  {
    for(uint8_t i=rf_grid_old; i<=num; i++)
    {
      if(i > 0)
      {
        oled_draw_grid(14+i, 21); 
      }
    }
  } 
  rf_grid_old = num;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Gray Scale Table Setting (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Gray_Scale_Table()
{
	spi_write_command(0xB8);			// Set Gray Scale Table
	spi_write_data(0x0C);			//   Gray Scale Level 1
	spi_write_data(0x18);			//   Gray Scale Level 2
	spi_write_data(0x24);			//   Gray Scale Level 3
	spi_write_data(0x30);			//   Gray Scale Level 4
	spi_write_data(0x3C);			//   Gray Scale Level 5
	spi_write_data(0x48);			//   Gray Scale Level 6
	spi_write_data(0x54);			//   Gray Scale Level 7
	spi_write_data(0x60);			//   Gray Scale Level 8
	spi_write_data(0x6C);			//   Gray Scale Level 9
	spi_write_data(0x78);			//   Gray Scale Level 10
	spi_write_data(0x84);			//   Gray Scale Level 11
	spi_write_data(0x90);			//   Gray Scale Level 12
	spi_write_data(0x9C);			//   Gray Scale Level 13
	spi_write_data(0xA8);			//   Gray Scale Level 14
	spi_write_data(0xB4);			//   Gray Scale Level 15

	spi_write_command(0x00);			// Enable Gray Scale Table
}

void Set_Linear_Gray_Scale_Table()
{
	spi_write_command(0xB9);			// Set Default Linear Gray Scale Table
}

/**************************************
  数据转换程序：将2位分成1个字节存入显存，由于1个seg表示4个列所以要同时写2个字节即4个像素
  uint8_t DATA：取模来的字模数据
****************************************/
void con_4_byte(uint8_t DATA)
{
   uint8_t d1_4byte[4],d2_4byte[4];
   uint8_t i;
   uint8_t d,k1,k2;
   d=DATA;
 
  for(i=0;i<2;i++)   // 一两位的方式写入  2*4=8位
   {
     k1=d&0xc0;     //当i=0时 为D7,D6位 当i=1时 为D5,D4位

     /****有4种可能，16级灰度,一个字节数据表示两个像素，一个像素对应一个字节的4位***/

     switch(k1)
       {
	 case 0x00:
           d1_4byte[i]=0x00;
		   delay_us(20);
         break;
     case 0x40:  // 0100,0000
           d1_4byte[i]=0x0f;
		   delay_us(20);
         break;	
	 case 0x80:  //1000,0000
           d1_4byte[i]=0xf0;
		   delay_us(20);
         break;
     case 0xc0:   //1100,0000
           d1_4byte[i]=0xff;
		   delay_us(20);
         break;	 
     default:
      	 break;
	   }
     
	   d=d<<2;
	  k2=d&0xc0;     //当i=0时 为D7,D6位 当i=1时 为D5,D4位

     /****有4种可能，16级灰度,一个字节数据表示两个像素，一个像素对应一个字节的4位***/

     switch(k2)
       {
	 case 0x00:
           d2_4byte[i]=0x00;
		   delay_us(20);
         break;
     case 0x40:  // 0100,0000
           d2_4byte[i]=0x0f;
		   delay_us(20);
         break;	
	 case 0x80:  //1000,0000
           d2_4byte[i]=0xf0;
		   delay_us(20);
         break;
     case 0xc0:   //1100,0000
           d2_4byte[i]=0xff;
		   delay_us(20);
         break;	 
     default:
      	 break;
	   }
	  
	  d=d<<2;                                //左移两位
      
	  
	  spi_write_data(d1_4byte[i]);                //写前2列
	  spi_write_data(d2_4byte[i]);                //写后2列	  共计4列
   }

}
/****************************************************
   写入一个16*16汉字
   x:列坐标，1个字节数据表示2列
   y: 行坐标
   a[32]：16点阵字模数据
   正常显示
******************************************************/
void hz_1616(uint8_t x,uint8_t y,uint8_t a[32])//write chinese word of1616
{
  uint16_t i,j;
  uint8_t temp;
  Set_Column_Address(Shift+x,Shift+x+3); // 设置列坐标，shift为列偏移量由1322决定。3为16/4-1
  Set_Row_Address(y,y+15);  //
  Set_Write_RAM();	 //	写显存
  
  for(j=0;j<16;j++)
  {
    for (i=0;i<2;i++) /* 2*8 column , a nibble of command is a dot*/
    {
      temp = a[(j<<1)+i];
      con_4_byte(temp);	 //数据转换
    }
  }
}
 
void oled_ascii_1608(uint8_t x, uint8_t y, uint8_t *str)
{
  uint8_t len = strlen((char const *)str);
  for(uint8_t i=0; i<len; i++)
  {
    ascii_1608(x+i*2, y, ASCII_1608[str[i]-0x20]);  
  }
}

void ascii_1608_dot(uint8_t x, uint8_t y, uint8_t *data)
{
  uint16_t j;
  uint8_t tmp;
  Set_Column_Address(Shift+x,Shift+x+1); // 设置列坐标，shift为列偏移量由1322决定。1为8/4-1
  Set_Row_Address(y,y+15);  //
  Set_Write_RAM();	 //	写显存
  
  for(j=0;j<16;j++)
  {
    tmp = data[j];
    if(j == 13)
    {
      tmp |= ASCII_1608['.'-0x20][j];
    }
    con_4_byte(tmp);	//数据转换
  }
}
void oled_freq_1608(uint8_t x, uint8_t y, uint8_t three, uint16_t two, uint8_t one, uint8_t all)
{
    uint8_t pos = x;
  uint8_t pos_y = y;
  if(mem_set_three != three || all)
  {
    
    if( three > 9 )
    {
      ascii_1608(pos, pos_y, ASCII_1608[three%100/10+16]);  // 16/4-1
    }else if(mem_set_three > 9 && mem_set_three != 0xff)
    {
      ascii_1608(pos, pos_y, ASCII_1608[' '-0x20]);  // 清除位
    }
    mem_set_three = three;
    pos += 2;
    ascii_1608(pos, pos_y, ASCII_1608[three%10+16]);
    pos += 2;
    ascii_1608(pos, pos_y, ASCII_1608['.'-0x20]);
    pos += 2;
  }else{
    pos += 6;
  }
  
  if(mem_set_two != two || all)
  {
    mem_set_two = two;
    
    ascii_1608(pos, pos_y, ASCII_1608[two%1000/100+16]);
    pos += 2;
    ascii_1608(pos, pos_y, ASCII_1608[two%100/10+16]);
    pos += 2;
    ascii_1608(pos, pos_y, ASCII_1608[two%10+16]);
    pos += 2;
    ascii_1608(pos, pos_y, ASCII_1608['.'-0x20]);
    pos += 2;
  }else{
    pos += 8;
  }
  
  if(mem_set_one != one || all)
  {
    mem_set_one = one;
    ascii_1608(pos, pos_y, ASCII_1608[one%100/10+16]);
    pos += 2;
    ascii_1608(pos, pos_y, ASCII_1608[one%10+16]) ;
  }
}

void oled_set_freq_1608(uint8_t x, uint8_t y, uint32_t freq)
{
  uint8_t pos = x;
  uint8_t pos_y = y;
  ascii_1608(pos, pos_y, ASCII_1608[freq%10000000/1000000+16]);  // 16/4-1
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608[freq%1000000/100000+16]);
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608['.'-0x20]);
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608[freq%100000/10000+16]);
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608[freq%10000/1000+16]);
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608[freq%1000/100+16]);
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608['.'-0x20]);
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608[freq%100/10+16]);
  pos += 2;
  ascii_1608(pos, pos_y, ASCII_1608[freq%10+16]);
}

void Show_CW_FRE_SET(uint8_t x, uint8_t y, uint32_t freq)
{
  uint8_t strbuf[16]="CW FREQ :";
  strbuf[9]=freq%10000/1000+'0';
  strbuf[10]=freq%1000/100+'0';
  strbuf[11]=freq%100/10+'0';
  strbuf[12]=freq%10+'0';
  strbuf[13]='H';
  strbuf[14]='z';
  
  Show_String0705(x, y, strbuf);
}

void Show_H_VSWR_P_SET(uint8_t x, uint8_t y, uint8_t val)
{
  uint8_t strbuf[12]="H-VSWR-P:";
  strbuf[9]=val%100/10+'0';
  strbuf[10]=val%10+'0';
  strbuf[11]=0;
  
  Show_String0705(x, y, strbuf);
}

void Show_CW_DELAY_SET(uint8_t x, uint8_t y, uint8_t sec)
{
  uint8_t strbuf[13]="CW DELAY:";
  strbuf[9]=sec%100/10+'0';
  strbuf[10]=sec%10+'0';
  strbuf[11]='S';
  strbuf[12]=0;
  
  Show_String0705(x, y, strbuf);
}

void Show_CALL_SET(uint8_t x, uint8_t y, uint8_t *code, uint8_t pos, uint8_t all)
{
  uint8_t pos_x = x;
  uint8_t pos_y = y;

  if( all )
  {
    oled_ascii_1608(pos_x, pos_y, "CALL:");
  }
  pos_x += 10;
  
  for( uint8_t i=0; i<MAX_CALLSIGN_LENGTH; i++)
  {
    if(pos == i || all)
    {
      ascii_1608(pos_x, pos_y, ASCII_1608[CALL_BUF[ code[i] ] -0x20]);
    }
    pos_x += 2;
  }
}

void Show_CALL_TYPE(uint8_t x, uint8_t y, uint8_t type)
{
  uint8_t pos_x = x;
  uint8_t pos_y = y;
   if( type )
   {
      ascii_1608(pos_x, pos_y, ASCII_1608['B' -0x20]);
   }
   else
   {
      ascii_1608(pos_x, pos_y, ASCII_1608['A' -0x20]);
   }
}

void Show_CW_MODE_SET(uint8_t x, uint8_t y, uint8_t mode)
{ 
  uint8_t strbuf_auto[]="CW MODE :AUTO  ";
  uint8_t strbuf_manual[]="CW MODE :MANUAL";
  
  if(mode == AUTO)
    Show_String0705(x, y, strbuf_auto);
  else
    Show_String0705(x, y, strbuf_manual);
}

void Show_CW_KEY_SET(uint8_t x, uint8_t y, uint8_t mode)
{ 
  uint8_t strbuf[15]="CW KEY  :MODE";
  strbuf[13]=mode+'0';
  strbuf[14]=0;
  
  Show_String0705(x, y, strbuf);
}

void Show_REF_SET(uint8_t x, uint8_t y, uint32_t freq)
{
  uint8_t strbuf[19]="OSC REF:";
  strbuf[8]=freq%100000000/10000000+'0';
  strbuf[9]=freq%10000000/1000000+'0';
  strbuf[10]='.';
  strbuf[11]=freq%1000000/100000+'0';
  strbuf[12]=freq%100000/10000+'0';
  strbuf[13]=freq%10000/1000+'0';
  strbuf[14]='.';
  strbuf[15]=freq%1000/100+'0';
  strbuf[16]=freq%100/10+'0';
  strbuf[17]=freq%10+'0';
  strbuf[18]=0;
  
  Show_String0705(x, y, strbuf);
}

void Show_R_BFO_SET(uint8_t x, uint8_t y, uint32_t freq)
{
  uint8_t strbuf[18]="CWR BFO:";
  strbuf[8]=freq%10000000/1000000+'0';
  strbuf[9]='.';
  strbuf[10]=freq%1000000/100000+'0';
  strbuf[11]=freq%100000/10000+'0';
  strbuf[12]=freq%10000/1000+'0';
  strbuf[13]='.';
  strbuf[14]=freq%1000/100+'0';
  strbuf[15]=freq%100/10+'0';
  strbuf[16]=freq%10+'0';
  strbuf[17]=0;
  
  Show_String0705(x, y, strbuf);
}

void Show_CW_VOL_SET(uint8_t x, uint8_t y, uint8_t duty)
{
  uint8_t strbuf[11]="CW  VOL:";
  strbuf[8]=duty%100/10+'0';
  strbuf[9]=duty%10+'0';
  strbuf[10]=0;
  
  Show_String0705(x, y, strbuf);
}

void Show_CWN_LO_SET(uint8_t x, uint8_t y, int16_t offset)
{
  uint8_t strbuf[14]="CWN  LO:";
  if (offset>=0)
    strbuf[8]='+';
  else
  {
    strbuf[8]='-';
    offset=-offset;
  }
  strbuf[9] =offset%10000/1000+'0';
  strbuf[10]=offset%1000/100+'0';
  strbuf[11]=offset%100/10+'0';
  strbuf[12]=offset%10+'0';
  strbuf[13]=0;
  
  Show_String0705(x, y, strbuf);
}
//貌似废弃  BG6RDF
void Show_CH_SET(uint8_t x, uint8_t y, uint8_t pos, uint32_t freq)
{
  uint8_t row[7][14];
  
  for(uint8_t i=0; i<7; i++)
  {
    row[i][0] = ASCII_0705['C'-0x20][i] << 3 | ASCII_0705['H'-0x20][i] >> 3 & 0x03 ;  // C(5) + 空 + H(2)
    row[i][1] = ASCII_0705['H'-0x20][i] << 5 | ASCII_0705[pos%100/10+16][i] >> 1;  // H(3) + 空 + 十位4)
    row[i][2] = ASCII_0705[pos%100/10+16][i] << 7 | ASCII_0705[pos%10+16][i] << 1; // 十位(1) + 空 + 个位(5) + 空 
    
    row[i][3] = ASCII_0705[':'-0x20][i] << 3 | ASCII_0705[freq%10000000/1000000+16][i] >> 1; // 空 + :(1) + 空(2) + 百万位(4) 
    row[i][4] = ASCII_0705[freq%10000000/1000000+16][i] << 7 | ASCII_0705[freq%1000000/100000+16][i] << 1; // 百万位(1) + 空 + 十万位(5) + 空 
    row[i][5] = ASCII_0705['.'-0x20][i] << 4 | ASCII_0705[freq%100000/10000+16][i] << 1; // .(1) + 空 +万位(5) + 空
    row[i][6] = ASCII_0705[freq%10000/1000+16][i] << 3 | ASCII_0705[freq%1000/100+16][i] >> 3; // 千位(5) + 空 + 百位(2) 
    row[i][7] = ASCII_0705[freq%1000/100+16][i] << 5 | ASCII_0705['.'-0x20][i] | ASCII_0705[freq%100/10+16][i] >> 3; // 百位(3) + 空 + .(1) + 空 + 十位(2) 
    row[i][8] = ASCII_0705[freq%100/10+16][i] << 5 | ASCII_0705[freq%10+16][i] >> 1; // 十位(3) + 空 + 个位(4)
    row[i][9] = ASCII_0705[freq%10+16][i] << 7 | ASCII_0705['H'-0x20][i] << 1; // 个位(1) + 空 + H(5) + 空
    row[i][10] = ASCII_0705['Z'-0x20][i] << 3; // Z(5) + 空(3)
  }
  
  Set_Column_Address(Shift+x,Shift+x+21);  // 11 * 8/4 - 1
  Set_Row_Address(y,y+7);
  Set_Write_RAM();
  
  for(uint8_t i=0; i<7; i++)
  {
    for(uint8_t j=0; j<11; j++)
    {
      con_4_byte(row[i][j]);  
    }
  }
}

void oled_mem_arrows_set(uint8_t position)
{
  if(arrows_pos <= 7)
  {
    Show_String0705(3, arrows_pos * 8, " ");
  }
  else
  {
    Show_String0705(30, (arrows_pos-8) * 8, " ");
  }
  arrows_pos = position % 16;
  
  if(arrows_pos <= 7)
  {
    Show_String0705(3, arrows_pos * 8, ">");
  }
  else
  {
    Show_String0705(30, (arrows_pos-8) * 8, ">");
  }

}

void oled_arrows_set_1608(uint8_t position)
{
  oled_ascii_1608(12, arrows_pos * 16, " ");
  arrows_pos = position % 4;
  
  oled_ascii_1608(12, arrows_pos * 16, ">");
}

void oled_menu_arrows_set(uint8_t position)
{
  // 清除 前一个位置的 > 号
  if(arrows_pos <= 4)
  {
    Show_String0705(3, arrows_pos * 8 + 12, " ");
  }
  else if(position <= 8)
  {
    Show_String0705(30, (arrows_pos-5) * 8 + 12, " ");
  }
  else if(position < SIZE_OF_MENU)
  {
    ascii_1608(14+(arrows_pos-9)*2, 40, ASCII_1608[' '-0x20]);
  }
  arrows_pos = position;
  
  // 显示 新位置的 > 号
  if(position <= 4)
  {
    Show_String0705(3, arrows_pos * 8 + 12, ">");
  }
  else if(position <= 8)
  {
    Show_String0705(30, (arrows_pos-5) * 8 + 12, ">");
  }
  else if(position < SIZE_OF_MENU)
  {
    ascii_1608(14+(arrows_pos-9)*2, 40, ASCII_1608['^'-0x20]);
  }
}

void oled_menu_set(uint8_t position)
{
  uint8_t pos_x = 5;
  uint8_t pos_y = 12;
  uint8_t all = 0 ;
  
  // 当菜单从第二屏 翻到 第一屏的时候，清屏及刷新第一屏数据
  if(position <= 8 && menu_pos >= 9)    //BG6RDF menu_pos是上次的位置
  {
    oled_set_screen(0x00);
    all = 1;
  }
  
  if( position == 0 || all)
  {
    Show_CW_KEY_SET(pos_x, pos_y, MENU.key);
  }
  pos_y += 8;
  
  if( position == 0 || position == 1 || all)
  {
    Show_CW_DELAY_SET(pos_x, pos_y, MENU.delay);
  }
  pos_y += 8;
  
  if( position == 0 || position == 2 || all)
  {
    Show_CW_FRE_SET(pos_x, pos_y, MENU.freqOfbuzzer);
    if((position != 0) && (!all))
    {
      buzzer_freq_set(MENU.freqOfbuzzer);
      sin_para_get();
      buzzer_start();
      QuietModeOn();
    }
  }
  else if( menu_pos == 2 && position != 2)
  {
    buzzer_stop();
    QuietModeOff();
  }
  pos_y += 8;
  
  if( position == 0 || position == 3 || all)
  {
    Show_CW_MODE_SET(pos_x, pos_y, MENU.cwMode);
  }
  pos_y += 8;
  
  if( position == 0 || position == 4 || all)
  {
    Show_H_VSWR_P_SET(pos_x, pos_y, MENU.vswr);
  }
  pos_y += 8;
  
  pos_x = 32;
  pos_y = 12;
  
  if( position == 0 || position == 5 || all)
  {
    Show_R_BFO_SET(pos_x, pos_y, MENU.rbfo);
  }
  pos_y += 8;
  
  if( position == 0 || position == 6 || all)
  {
    Show_CW_VOL_SET(pos_x, pos_y, MENU.dutyOfbuzzer);
    if((!all) && (position != 0))
    {
      sin_para_get();
      buzzer_start();
      QuietModeOn();
    }
  }
  else if( menu_pos == 6 && position != 6)
  {
    buzzer_stop();
    QuietModeOff();
  }
  pos_y += 8;
  
  //  if( position == 0 || position == 6 || all)
  //  {
  //  Show_T_BFO_SET(pos_x, pos_y, MENU.tbfo);
  //  }
  //  pos_y += 8;
  //  
  //  if( position == 0 || position == 7 || all)
  //  {
  //  Show_USB_BFO_SET(pos_x, pos_y, MENU.usbbfo);
  //  }
  //  pos_y += 8;
  //  
  //  if( position == 0 || position == 8 || all)
  //  {
  //  Show_LSB_BFO_SET(pos_x, pos_y, MENU.lsbbfo);
  //  }
  //  pos_y += 8;
  
  if( position == 0 || position == 7 || all)
    Show_REF_SET(pos_x, pos_y, MENU.ref);
  pos_y += 8;
  
  if (position ==0 || position == 8 || all)
    Show_CWN_LO_SET(pos_x, pos_y, MENU.cwnloOffset);
  pos_y += 8;

  pos_x = 4;
  pos_y = 24;
  // 当菜单从第一屏 翻到 第二屏的时候，清屏及刷新第二屏数据
  if((position >= 9 && menu_pos <= 8))
  {
    oled_set_screen(0x00);
    Show_CALL_SET(pos_x, pos_y, &MENU.callSign[0], position - 8, 1);
  }
  else if(position >= 9)
  {
    Show_CALL_SET(pos_x, pos_y, &MENU.callSign[0], position - 8, 0);
  }
  //menu_pos记录上一次显示的是哪个菜单
  menu_pos = position;
}


//void oled_mem_set_0705(uint8_t position)
//{
//  uint8_t pos = position / 16;
//  uint8_t pos_x = 5;
//  uint8_t pos_y = 0;
//  pos <<= 4;
//  
//  
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  
//  pos_x = 32;
//  pos_y = 0;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  Show_CH_SET(pos_x, pos_y, pos, MEM.val[pos]);
//  pos_y += 8;
//  pos ++;
//  
//}

void oled_lsb_usb_mem(uint8_t *str)
{
  oled_ascii_1608(48, 24, str);
}

void oled_h_vswr_disp()
{
  Show_String0705(10, 48, "HI-SWR");
}

void oled_h_vswr_clear()
{
  Show_String0705(10, 48, "      ");
}

void oled_mem_set(uint8_t position, uint32_t freq, uint8_t all)
{
  uint8_t pos_x = 8;
  uint8_t pos_y = 24;
  uint8_t ten = position % 100 / 10;
  uint8_t ge = position % 10;
  
  if( all )
  {
    oled_ascii_1608(pos_x, pos_y, "CH");
  }
  pos_x += 4;
  ascii_1608(pos_x, pos_y, ASCII_1608[ten+16]);
  pos_x += 2;
  ascii_1608(pos_x, pos_y, ASCII_1608[ge+16]);
  pos_x += 2;
  if( all )
  {
    oled_ascii_1608(pos_x, pos_y, ":");
  }
  pos_x += 2;
  oled_freq_1608(pos_x, pos_y, freq%10000000/100000, freq%100000/100 ,freq%100, all);
  pos_x += 18;
  if( all )
  {
    oled_ascii_1608(pos_x, pos_y, "HZ");
  }
}

 /****************************************************
   写入一个8*16 ascii
   x:列坐标，
   y: 行坐标
   a[16]：16点阵字模数据
   正常显示 
******************************************************/
void ascii_1608(uint8_t x,uint8_t y,uint8_t a[16] )
{
  uint16_t j;
  Set_Column_Address(Shift+x,Shift+x+1); // 设置列坐标，shift为列偏移量由1322决定。1为8/4-1
  Set_Row_Address(y,y+15);  //
  Set_Write_RAM();	 //	写显存
  
  for(j=0;j<16;j++)
  {
    con_4_byte(a[j]);	//数据转换
  }
}

void oled_ascii_3216_dot(uint8_t x, uint8_t y, uint8_t *data)
{
  uint8_t temp;
  Set_Column_Address(Shift+x,Shift+x+3); // 设置列坐标，shift为列偏移量由1322决定。3为16/4-1
  Set_Row_Address(y,y+31);  //
  Set_Write_RAM();	 //	写显存
  
  for(uint8_t j=0;j<32;j++)
  {
    for (uint8_t i=0;i<2;i++) /* 2*8 column , a nibble of command is a dot*/
    {
      temp = data[(j<<1)+i];
      if( ((j<<1)+i)==49 || ((j<<1)+i)==51)
      {
        temp |= DIGIT_3216[10][(j<<1)+i];	 //数据转换
      }
      con_4_byte(temp);
    }
  }
}

void oled_ascii_3216(uint8_t x, uint8_t y, uint8_t *data)
{
  uint8_t temp;
  Set_Column_Address(Shift+x,Shift+x+3); // 设置列坐标，shift为列偏移量由1322决定。3为16/4-1
  Set_Row_Address(y,y+31);  //
  Set_Write_RAM();	 //	写显存
  
  for(uint8_t j=0;j<32;j++)
  {
    for (uint8_t i=0;i<2;i++) /* 2*8 column , a nibble of command is a dot*/
    {
      temp = data[(j<<1)+i];
      con_4_byte(temp);	 //数据转换
    }
  }
}

void oled_draw_scale(uint8_t x, uint8_t y)
{
  Set_Column_Address(Shift+x,Shift+x+29); // 设置列坐标，shift为列偏移量由1322决定。29为120/4-1
  Set_Row_Address(y,y+11);  //
  Set_Write_RAM();	 //	写显存
  
  for(uint16_t j=0;j<165;j++)
  {
      con_4_byte(scaler_rx[j]);	 //数据转换
  }
}