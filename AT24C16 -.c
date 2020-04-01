#include <reg52.h>
#include <intrins.h>
#include <Absacc.h>
#include <string.h>
#include <ctype.h>

#define byte unsigned char
#define uchar unsigned char
#define word unsigned int
#define uint unsigned int
#define ulong unsigned long
#define BYTE 	unsigned char
#define WORD 	unsigned int

#define True 		0xff
#define False 		0x00

#define   High     1
#define   Low      0

sbit WP		=		P3^4;
sbit SDA	=		P3^1;
sbit SCL	=		P3^0;
#define SDA0		SDA=0
#define SDA1		SDA=1
#define SCL1		SCL=1
#define SCL0		SCL=0

#define LOW			0
#define HIGH		1

extern void delay_5us(void);//延时5us，晶振改变时只用改变这一个函数！
extern void time(unsigned int ucMs);//延时单位：ms

//仅仅在此模块内调用的函数!
void I_stop(void);
void I_init(void);
//void STA(void);
byte RDBYT(void);
byte CACK(void);
void WRBYT(byte aa);
bit I_clock(void);
bit I_send(BYTE I_data);
bit E_address(BYTE ucAddress,BYTE Write);
void I_ack(void);

byte ReadOne(word CatAddr);
bit  WriteOne(word CatAddr,byte WB);

void Read24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData);
//任意地址,任意长度!

bit Write24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData);
//单个字节写,速度慢!返回1成功，返回0失败！

bit PageWrite24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData); 
//wStartAddr的低4位须为0000,每次写16个字节,速度快!返回1成功，返回0失败！

/****************************************************************************
*    函数原型: void I_stop(void) 
*    功    能: 提供I2C总线工作时序中的停止位                              
****************************************************************************/
void I_stop(void)
{
       SDA=LOW;
       SCL=HIGH;
       delay_5us();
       SDA=HIGH;
       delay_5us();
       SCL=LOW;
       delay_5us();
}
/****************************************************************************
*    函数原型: void I_init(void) 
*    功    能: I2C总线初始化。在main()函数中应首先调用本函数, 然后再调用
*              其他函数 
****************************************************************************/
void I_init(void)
{
       SCL0;
       I_stop();
}
/****************************************************************************
*    函数原型: void I_start(void) 
*    功    能: 提供I2C总线工作时序中的起始位                              
****************************************************************************/
void I_start(void)
{
       SDA=HIGH;
       delay_5us();
       SCL=HIGH;
       delay_5us();
       SDA=LOW;
       delay_5us();
       SCL=LOW;
}
/****************************************************************************
*    函数原型: BYTE I_receive(void) 
*    功    能: 从I2C总线上接收8位数据信号, 并将接收到8位数据作为1B
*              返回, 不回送应答信号ACK。主函数在调用本函数之前应保证SDA信
*              号线处于浮置状态, 即使89C52的P1.7脚置1
****************************************************************************/
BYTE I_receive(void)
{
      register BYTE   i;
      BYTE I_data=0;
      for(i=0;i<8;i++)
      {
            I_data<<=1;
            if(I_clock())
                 I_data++;
      }
      return(I_data);
}
/************* RDBYT *************/
/* from i2c bus read a byte*/
byte RDBYT(void)
{
	byte i;
	byte aa;
	aa=0;
	for(i=0;i<8;i++)
	{
		delay_5us();
		SDA1;
		delay_5us();
		SCL1;
		delay_5us();
		aa=aa<<1;
		if(SDA==1)aa|=1;
		SCL0;
	}
	return(aa);
}
/************** CACK *******************/
/* check ack */
byte CACK(void)
{
	SDA1;
	delay_5us();
	SCL1;
	delay_5us();
	if(SDA==1)
	{
		SCL0;
		return(1);
	}
	else
	{
		SCL0;
		return(0);
	}
}
/************** WRBYT **************/
/* write i2c bus a byte */
void WRBYT(byte aa)
{
	register byte i;
	byte ra;
	for(i=0;i<8;i++)
	{
		delay_5us();
		ra=aa&0x80;
		if(ra==0x80)
			SDA1;
		else
			SDA0;
		SCL1;
		delay_5us();
		SCL0;
		aa=_crol_(aa,1);
	}
}
/************* ReadOne *********************/
byte ReadOne(word CatAddr)
{
	byte rb;
	byte CatAddrH,CatAddrL;

	byte page;
	CatAddrH=(byte)(CatAddr>>8);
	CatAddrL=(byte)CatAddr;
	page=CatAddrH<<1;
	I_start();
	WRBYT(0xa0+page);
	if(CACK()==1)
	{
		I_stop();
		return(0xff);
	}
	WRBYT(CatAddrL);
	if(CACK()==1)
	{
		I_stop();
		return(0xff);
	}
	I_start();
	WRBYT(0xa1+page);
	if(CACK()==1)
	{
		I_stop();
		return(0xff);
	}
	rb=RDBYT();
	SDA1;
	I_stop();
	return(rb);
}
/**************************************************************
* 函数说明：写数据到24C021的某一地址中
* 入口参数：
     wStartAddr:地址
     ucData:  数据
* 返回：
     1：成功
     0：失败
* 作者：
* 创建日期：20030517
**************************************************************/
bit  WriteOne(WORD wStartAddr,BYTE ucData)
{
BYTE    idata Write;

	if(ReadOne(wStartAddr)==ucData)
        	return(1);
        
        Write=0xA0+(((BYTE)(wStartAddr>>8))<<1);

      WP=0;
      I_init();
      if(E_address(wStartAddr,Write)&&I_send(ucData))
      {
         I_stop();
         time(10);
      }
      else
      {
          WP=1;
	  return(0);
      }
      WP=1;
      return(1);
}
/****************************************************************************
*    函数原型: bit  I_clock(void) 
*    功    能: 提供I2C总线的时钟信号, 并返回在时钟电平为高期间SDA信号线上状
*              态。本函数可用于数据发送, 也可用于数据接收 
****************************************************************************/
bit I_clock(void)
{
       SCL=HIGH;
       delay_5us();
       if(SDA)
       {
          SCL=LOW;
          delay_5us();
          return(1);
       }
       SCL=LOW;
       delay_5us();
       return(0);
}
/********************************************************
*    函数原型: bit I_send(BYTE I_data)
*    功    能: 向I2C总线发送8位数据, 并请求一个应答信号ACK。
	 如果收到ACK应答则返回1(TRUE), 否则返回0(FALSE) 
**********************************************/
bit I_send(BYTE I_data)
{
      register BYTE i;
      for(i=0;i<8;i++)
      {
           SDA=(bit)(I_data&0x80);
           I_data<<=1;
           I_clock();
      }
      SDA=High;
      return(~I_clock());

}
/****************************************************************************
*    函数原型: bit E_address(BYTE ucAddress, Write)
*    功    能: 向24C04写入器件地址和一个指定的字节
*****************************************************************************/
bit E_address(BYTE ucAddress,BYTE Write)
{
      I_start();
      if(I_send(Write))
          {return(I_send(ucAddress));}
      else
          {return(0);}
}
/****************************************************************************
*    函数原型: void I_Ack(void);
*    功    能: 向I2C总线发送一个应答信号ACK, 一般用于连续数据读取时
*****************************************************************************/
void I_ack(void)
{
       SDA0;
       delay_5us();
       SCL1;
       delay_5us();
       SCL0;
       delay_5us();
       SDA1;
}
/******************************************************* 
* 函数说明： 从24C021读一组数
* 入口参数： 
*    wStartAddr：首地址
*    ucLen：     数据长度
*    ucpData：   数据体指针
* 返回：    无
* 作者：   
* 创建日期：20030517
*******************************************************/
void Read24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData)
{
BYTE i;
BYTE    idata Write,Read;
	Write=0xA0+(((BYTE)(wStartAddr>>8))<<1);
	Read=Write+1;
	
    I_init();
    if(E_address(wStartAddr,Write)) {    
		  I_start();delay_5us();
          if(I_send(Read)){
  	     		for(i=0;i<ucLen;i++) {
  	        		ucpData[i]=I_receive();
           			if (i!=(ucLen-1)) I_ack();
				}
             	I_stop();
  	       }	  
    }
}
/******************************************************* 
* 函数说明： 向24C021写一组数
* 入口参数： 
* 入口参数： 
*    wStartAddr：首地址
*    ucLen：     数据长度
*    ucpData：   数据体指针
* 返回：    
    1：成功
    0：不成功
* 作者：   
* 创建日期：20030517
*******************************************************/
bit Write24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData)
{
	BYTE i;
	for(i=0;i<ucLen;i++)
	{
		if(!WriteOne(wStartAddr+i,ucpData[i]))
		{
			return(0);
		}
	}
	return(1);
}
/******************************************************* 
* 函数说明： 向24C021写16个字节
* 入口参数： 
* 入口参数： 
*    wStartAddr：首地址 //最多写16个字节，开始地址为XXX0
*    ucLen：     数据长度
*    ucpData：   数据体指针
* 返回：    
    1：成功
    0：不成功
* 作者：   
* 创建日期：20030517
*******************************************************/
bit PageWrite24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData)
{	
BYTE i;
BYTE  idata Write;

        Write=0xA0+(((BYTE)(wStartAddr>>8))<<1);
      
      WP=0;
      I_init();
      
      if(!E_address(wStartAddr,Write))
      {
          WP=1;
	  return(0);
      }
      
      for(i=0;i<ucLen;i++) {
         if(!I_send(ucpData[i])) {
                I_stop();
          	WP=1;
	 	return(0);
         }         
      }
      I_stop();
      time(12);
      WP=1;
      return(1);
}
