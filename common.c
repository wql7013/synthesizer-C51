#include <REG52.H>      /* special function register declarations */
#include <intrins.h>
/*********************************************************
*  函数说明：延时5us，晶振改变时只用改变这一个函数！
	1、对于11.0592M晶振而言，需要2个_nop_();
	2、对于22.1184M晶振而言，需要4个_nop_();
*  入口参数：无 
*  返回：    无
*********************************************************/
void delay_5us(void)//延时5us，晶振改变时只用改变这一个函数！
{
	_nop_();
	_nop_();
	//_nop_();
	//_nop_();
}
/************ delay_50us ***************/
void delay_50us(void)//延时50us
{
	unsigned char i;
	for(i=0;i<4;i++)
	{
		delay_5us();
	}
}
/******** 延时100us ******************/
void delay_100us(void)//延时100us
{
	delay_50us();
	delay_50us();
}

/*********** 延时单位：ms *******************/
void time(unsigned int ucMs)//延时单位：ms
{
	unsigned char j;	
	while(ucMs>0){
		for(j=0;j<10;j++) delay_100us();
		ucMs--;
	}
}
/********** 初始化串口波特率 ************/
void initUart(void)/*初始化串口波特率，使用定时器2*/
{
/* Setup the serial port for 9600 baud at 11.0592MHz */
	SCON = 0x50;  //串口工作在方式1
	RCAP2H=(65536-(3456/96))>>8;
	RCAP2L=(65536-(3456/96))%256;
  	T2CON=0x34;
    TI  = 1;     /* 置位TI*/
}
