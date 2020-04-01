
/********************************************************************************
*  描述:                                                                       *    
*        矩阵键盘1602液晶显示键值                                              *    
*   ********************************************************************************/


#include <common.h>
	                
#define uchar unsigned char
#define uint  unsigned int
#define DATA P1

void delay0(uchar x);      //x*0.14MS

sbit LCD_RS = P3^5;
sbit LCD_RW = P3^6;
sbit LCD_EN = P3^7;

void lcd_init();
void lcd_clear();
void lcd_wdat(uchar dat);
void lcd_pos(uchar pos);

#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};

/*************************************************************/
/*                                                           */
/* 延时子程序                                                */
/*                                                           */
/*************************************************************/

void  delay(uchar x)
{ uchar j;
    while((x--)!=0)
    { for(j=0;j<5;j++)
         {;}
    }
}

/*************************************************************/
/*                                                           */
/*检查LCD忙状态                                              */
/*lcd_busy为1时，忙，等待。lcd-busy为0时,闲，可写指令与数据  */
/*                                                           */
/*************************************************************/ 

bit lcd_busy()
 {                          
    bit result;
    LCD_RS = 0;
    LCD_RW = 1;
    LCD_EN = 0;
    delay(10);
    LCD_EN = 1;
	 delay(50);
    result = (bit)(DATA&0x80);
    return(result);
 }

/*******************************************************************/
/*                                                                 */
/*写指令数据到LCD                                                  */
/*RS=L，RW=L，E=高脉冲，D0-D7=指令码。                             */
/*                                                                 */
/*******************************************************************/

void lcd_wcmd(uchar cmd)

{                          
   while(lcd_busy());
    LCD_RS = 0;
    LCD_RW = 0;
    //LCD_EN = 0;
    _nop_();
    _nop_(); 
    DATA = cmd;
    delay(50);
    LCD_EN = 1;
    delay(50);
    LCD_EN = 0;
    delay(50);
	DATA=0xFF;
}

/*******************************************************************/
/*                                                                 */
/*写显示数据到LCD                                                  */
/*RS=H，RW=L，E=高脉冲，D0-D7=数据。                               */
/*                                                                 */
/*******************************************************************/

void lcd_wdat(uchar dat)
{                      
   while(lcd_busy());
    LCD_RS = 1;
    LCD_RW = 0;
    //LCD_EN = 0;
    DATA = dat;
    delay(50);
    LCD_EN = 1;
    delay(50);
    LCD_EN = 0; 
    delay(50);
	DATA=0xFF;
}

/*************************************************************/
/*                                                           */
/*  LCD初始化设定                                            */
/*                                                           */
/*************************************************************/

void lcd_init()
{
    DATA=0xFF;
    delay(200);                   
    lcd_wcmd(0x30);      
    delay(100);
    lcd_wcmd(0x30);         
    delay(100);
    lcd_wcmd(0x30);         
    delay(100);

    lcd_wcmd(0x38);      //16*2显示，5*7点阵，8位数据 
	delay(50);
	lcd_wcmd(0x08);      //显示关，光标关 
    delay(50);
    lcd_wcmd(0x01);      //清除LCD的显示内容   
    delay(50);
    lcd_wcmd(0x06);      //增量方式，不移位   
	delay(50);
    lcd_wcmd(0x0C);           //开显示      
}

/*************************************************************/
/*                                                           */
/*  设定显示位置                                             */
/*                                                           */
/*************************************************************/

void lcd_pos(uchar pos)
{                          
  lcd_wcmd(pos | 0x80);  //数据指针=80+地址变量    
}
