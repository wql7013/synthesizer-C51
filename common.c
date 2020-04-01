#include <REG52.H>      /* special function register declarations */
#include <intrins.h>
/*********************************************************
*  ����˵������ʱ5us������ı�ʱֻ�øı���һ��������
	1������11.0592M������ԣ���Ҫ2��_nop_();
	2������22.1184M������ԣ���Ҫ4��_nop_();
*  ��ڲ������� 
*  ���أ�    ��
*********************************************************/
void delay_5us(void)//��ʱ5us������ı�ʱֻ�øı���һ��������
{
	_nop_();
	_nop_();
	//_nop_();
	//_nop_();
}
/************ delay_50us ***************/
void delay_50us(void)//��ʱ50us
{
	unsigned char i;
	for(i=0;i<4;i++)
	{
		delay_5us();
	}
}
/******** ��ʱ100us ******************/
void delay_100us(void)//��ʱ100us
{
	delay_50us();
	delay_50us();
}

/*********** ��ʱ��λ��ms *******************/
void time(unsigned int ucMs)//��ʱ��λ��ms
{
	unsigned char j;	
	while(ucMs>0){
		for(j=0;j<10;j++) delay_100us();
		ucMs--;
	}
}
/********** ��ʼ�����ڲ����� ************/
void initUart(void)/*��ʼ�����ڲ����ʣ�ʹ�ö�ʱ��2*/
{
/* Setup the serial port for 9600 baud at 11.0592MHz */
	SCON = 0x50;  //���ڹ����ڷ�ʽ1
	RCAP2H=(65536-(3456/96))>>8;
	RCAP2L=(65536-(3456/96))%256;
  	T2CON=0x34;
    TI  = 1;     /* ��λTI*/
}
