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

extern void delay_5us(void);//��ʱ5us������ı�ʱֻ�øı���һ��������
extern void time(unsigned int ucMs);//��ʱ��λ��ms

//�����ڴ�ģ���ڵ��õĺ���!
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
//�����ַ,���ⳤ��!

bit Write24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData);
//�����ֽ�д,�ٶ���!����1�ɹ�������0ʧ�ܣ�

bit PageWrite24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData); 
//wStartAddr�ĵ�4λ��Ϊ0000,ÿ��д16���ֽ�,�ٶȿ�!����1�ɹ�������0ʧ�ܣ�

/****************************************************************************
*    ����ԭ��: void I_stop(void) 
*    ��    ��: �ṩI2C���߹���ʱ���е�ֹͣλ                              
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
*    ����ԭ��: void I_init(void) 
*    ��    ��: I2C���߳�ʼ������main()������Ӧ���ȵ��ñ�����, Ȼ���ٵ���
*              �������� 
****************************************************************************/
void I_init(void)
{
       SCL0;
       I_stop();
}
/****************************************************************************
*    ����ԭ��: void I_start(void) 
*    ��    ��: �ṩI2C���߹���ʱ���е���ʼλ                              
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
*    ����ԭ��: BYTE I_receive(void) 
*    ��    ��: ��I2C�����Ͻ���8λ�����ź�, �������յ�8λ������Ϊ1B
*              ����, ������Ӧ���ź�ACK���������ڵ��ñ�����֮ǰӦ��֤SDA��
*              ���ߴ��ڸ���״̬, ��ʹ89C52��P1.7����1
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
* ����˵����д���ݵ�24C021��ĳһ��ַ��
* ��ڲ�����
     wStartAddr:��ַ
     ucData:  ����
* ���أ�
     1���ɹ�
     0��ʧ��
* ���ߣ�
* �������ڣ�20030517
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
*    ����ԭ��: bit  I_clock(void) 
*    ��    ��: �ṩI2C���ߵ�ʱ���ź�, ��������ʱ�ӵ�ƽΪ���ڼ�SDA�ź�����״
*              ̬�����������������ݷ���, Ҳ���������ݽ��� 
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
*    ����ԭ��: bit I_send(BYTE I_data)
*    ��    ��: ��I2C���߷���8λ����, ������һ��Ӧ���ź�ACK��
	 ����յ�ACKӦ���򷵻�1(TRUE), ���򷵻�0(FALSE) 
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
*    ����ԭ��: bit E_address(BYTE ucAddress, Write)
*    ��    ��: ��24C04д��������ַ��һ��ָ�����ֽ�
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
*    ����ԭ��: void I_Ack(void);
*    ��    ��: ��I2C���߷���һ��Ӧ���ź�ACK, һ�������������ݶ�ȡʱ
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
* ����˵���� ��24C021��һ����
* ��ڲ����� 
*    wStartAddr���׵�ַ
*    ucLen��     ���ݳ���
*    ucpData��   ������ָ��
* ���أ�    ��
* ���ߣ�   
* �������ڣ�20030517
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
* ����˵���� ��24C021дһ����
* ��ڲ����� 
* ��ڲ����� 
*    wStartAddr���׵�ַ
*    ucLen��     ���ݳ���
*    ucpData��   ������ָ��
* ���أ�    
    1���ɹ�
    0�����ɹ�
* ���ߣ�   
* �������ڣ�20030517
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
* ����˵���� ��24C021д16���ֽ�
* ��ڲ����� 
* ��ڲ����� 
*    wStartAddr���׵�ַ //���д16���ֽڣ���ʼ��ַΪXXX0
*    ucLen��     ���ݳ���
*    ucpData��   ������ָ��
* ���أ�    
    1���ɹ�
    0�����ɹ�
* ���ߣ�   
* �������ڣ�20030517
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
