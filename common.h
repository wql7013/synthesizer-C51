#include <REG52.H>    /* special function register declarations */
#include <stdio.h>    /* prototype declarations for I/O functions */
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

#define TRUE  1
#define FALSE 0

extern void initUart(void);/*初始化串口*/
extern void time(unsigned int ucMs);//延时单位：ms