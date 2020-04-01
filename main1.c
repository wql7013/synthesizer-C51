/**********************************
功能:播放音乐、录制音乐、弹奏歌曲

预设天空之城、罗密欧与朱丽叶

最多可保存8首用户录制音乐
每首音乐最多为24个音阶(一个休止符算一个音阶)

最佳晶振频率为12MHz
*************************************/
#include "common.h"
#define KEY (~P2&0x3F)
#define LCD_CLEAR() lcd_wcmd(0x01)
#define LCD_INPUT_START() lcd_wcmd(0x0F)
#define LCD_INPUT_OK() lcd_wcmd(0x0C)
#define LCD_INPUT_FLASH() lcd_wcmd(0x13)
#define LCD_SCR_LMOV() lcd_wcmd(0x1B)
#define LCD_SCR_RMOV() lcd_wcmd(0x1F)
sbit music=P2^6;
sbit SHIFT=P0^0;
sbit BACK=P0^1;
sbit FORE=P0^2;
sbit NEXT=P0^3;
sbit OK=P0^4;
sbit CLREROM=P0^5;
sbit PAUSE=P0^6;

//Octavec1储存两个八度里面的24个音对应的定时器初值，0表示休止符
//score1储存天空之城简谱每个音对应Octavec1的下标
//meter1储存天空之城节拍，8为一个全音符
unsigned char count1=0,count2=0,i=0;
uchar code Octavec1[]={0,17,31,43,55,66,77,87,97,106,114,122,129,
                           137,143,150,156,161,167,172,176,181,185,189,193};
uchar code score1[]={10,12,13,12,13,17,12,0,5,10,8,10,13,8,0,5,5,6,
                        5,6,13,5,0};
uchar code meter1[]={2,2,6,2,4,4,8,4,4,6,2,4,4,8,2,2,2,6,2,4,4,8,4};
uchar code len1=23;
uchar code name1[]="Laputa";

uchar code score2[]={10,13,12,5,5,8,5,10,10,8,6,8,8,6,5,3,5,3,1,3,5,0};
uchar code meter2[]={ 2, 2, 2,6,2,2,2, 6, 2,2,2,6,2,2,2,6,1,1,2,2,6,8};
uchar code len2=22;
uchar code name2[]="Rom&Juli";

uchar code str_nor[]="No record!";
uchar code str_play[]="user playing";
uchar code str_record[]="user recording.";
uchar code str_overflow[]="overflow,restart or save";
uchar code str_save[]="record saved";
uchar code str_error[]="access error!";
uchar code str_input[]="input music name:";
uchar code str_memf[]="memory full!";
uchar code str_delf[]="failed!memory cleared!reset!";
uchar code str_dels[]="delete sucessfully!";
uchar code str_pause[]="pause";
uchar code str_clrerom[]="EEPROM cleared!";
uchar code str_wcome[]="Welcome!designed by wql";

uchar score[24]=0;
uchar meter[24]=1;
uchar name[10]=0;
uchar len=0;
uchar id=0;
uchar tolnum=0;
uchar status=0;
//EEPROM函数
void I_init(void);
bit PageWrite24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData);
byte ReadOne(word CatAddr);
bit  WriteOne(word CatAddr,byte WB);
void Read24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData);
bit Write24C16(WORD wStartAddr,BYTE ucLen,BYTE *ucpData);
//LCD函数
void lcd_init();
void lcd_wdat(uchar dat);
void lcd_pos(uchar pos);
void lcd_wcmd(uchar);

void music_disp(void);

void main(void)
{
  uchar k;

  ET0=1;ET1=1;PT0=1;PT1=1;
  TMOD=0x12;
  EX0=1;EA=1;
  IT0=1;IT1=1;
  I_init();
  lcd_init();

  k=0;
  while(str_wcome[k]!=0)
  {
    lcd_wdat(str_wcome[k++]);
	 if(k>0x0F)
	  lcd_pos(0x30+k);
  }
  time(2000);

  if(ReadOne(0x7FF)!=0xA5)
  {
    EA=0;
	 for(k=0;k<8;k++)
     if(~WriteOne(k<<4,0))
	  {
	    LCD_CLEAR();
	    k=0;
	    while(str_error[k]!=0)
        lcd_wdat(str_error[k++]);
   	 while(1);
	  }
	 if(~WriteOne(0x7FF,0xA5))
	 {
	    LCD_CLEAR();
	    k=0;
	    while(str_error[k]!=0)
        lcd_wdat(str_error[k++]);
   	 while(1);
	  }
	 EA=1;
  }

  len=len1;
  for(k=0;k<len;k++)
  {
    score[k]=score1[k];
    meter[k]=meter1[k];
  }
  for(k=0;name1[k]!=0;k++)
   name[k]=name1[k];
  name[k]=0;
  id=1;
  tolnum=2;

  music_disp();
  TH0=Octavec1[score[0]];TL0=TH0;
  TL1=0;TH1=0;
  TR0=1;TR1=1;
  while(1)
  {
   if(CLREROM==0)
   {
     EA=0;
	  for(k=0;k<8;k++)
      if(~WriteOne(k<<4,0))
	   {
		  LCD_CLEAR();
	     k=0;
		  while(str_error[k]!=0)
         lcd_wdat(str_error[k++]);
		  WriteOne(0x7FF,0xFF);
		  while(1);
	   }
	  if(status==0||status==2)
	  {
		 lcd_pos(17);
	    for(k=0;k<16;k++)
		 {
	      LCD_SCR_RMOV();
         lcd_wdat(0);
		 }
		 lcd_pos(17);
	  }
	  else
		LCD_CLEAR();
     k=0;
	  while(str_clrerom[k]!=0)
      lcd_wdat(str_clrerom[k++]);
	  time(1000);
     while(CLREROM==0);
	  if(status==0||status==2)
		for(k=0;k<16;k++)
		 LCD_SCR_LMOV();
	  else
	  {
		 TR1=0;TR2=0;
	  }
	  EA=1;
   }
	if(PAUSE==0)
	{
	  k=(uchar)TR1<<1;TR1=0;
	  k=k+(uchar)TR0;TR0=0;
	  EA=0;
	  while(PAUSE==0);
	  EA=1;
	  TR0=(bit)k&0x01;
	  TR1=(bit)(k>>1);
	}
  }
}

void scoref(void) interrupt 1
{
  count1++;
  if(count1==4)
  {
    if(score[i]!=0)
     music=~music;
    count1=0;
  }
}
void meterf(void) interrupt 3
{ 
  unsigned char k,j;

  if(count2<255)
   count2++;
  if(status==3)
  {
    TL1=0;TH1=0;
	 return;
  }
  if(count2==meter[i])
  {
    TR0=0;
    if(i==len-1)
     i=0;
	 else
	  i++;
    TL0=Octavec1[score[i]];TH0=Octavec1[score[i]];
    count2=0;
    for(k=0;k<255;k++)
     for(j=0;j<40;j++);
    TR0=1;
  }
  TL1=0;TH1=0;
}

void FUNCTION(void) interrupt 0
{
  uchar k,t;
  switch(KEY&0x07)
  {
    case 0:
         TR0=0;TR1=0;

         len=len1;
         for(k=0;k<len;k++)
         {
           score[k]=score1[k];
           meter[k]=meter1[k];
         }
         for(k=0;name1[k]!=0;k++)
          name[k]=name1[k];
         name[k]=0;
         id=1;
         tolnum=2;

         music_disp();
         TH0=Octavec1[score[0]];TL0=TH0;
         count1=0;count2=0;i=0;
         TL1=0;TH1=0;
         TR0=1;TR1=1;
         EX1=0;
         status=0;
         break;
	 case 1:
         TR0=0;TR1=0;

         t=tolnum;
			tolnum=0;
         for(k=0;k<8;k++)
          if(ReadOne(k<<4)!=0)
	        tolnum++;
         if(tolnum==0)
			{
			  lcd_pos(17);
	        for(k=0;k<16;k++)
		     {
	          LCD_SCR_RMOV();
             lcd_wdat(0);
		     }
			  lcd_pos(17);
           k=0;
           while(str_nor[k]!=0)
            lcd_wdat(str_nor[k++]);
			  time(2000);
			  for(k=0;k<16;k++)
			   LCD_SCR_LMOV();
			  tolnum=t;
			  if(status!=5)
			  {
			    TR0=1;TR1=1;
			  }
			  break;
			}
         
			id=1;
         for(k=0;k<8;k++)
          if(ReadOne(k<<4)==tolnum-id+1)
	       {
	         len=ReadOne((k<<4)+1);
	         t=0;
            while((name[t]=ReadOne((k<<4)+2+t))!=0)
	          t++;
	         Read24C16(((uint)k<<5)+0x100,len,score);
            Read24C16(((uint)k<<5)+0x200,len,meter);
				break;
	       }

         music_disp();
         TH0=Octavec1[score[0]];TL0=TH0;
         count1=0;count2=0;i=0;
         TL1=0;TH1=0;
         TR0=1;TR1=1;
         EX1=0;
         status=1;
			break;
	 case 2:
         switch(status)
         {
           case 0:
	             if(id<tolnum)
                {
                  TR0=0;TR1=0;
                  id++;
			         len=len2;
			         for(k=0;k<len;k++)
                  {
                    score[k]=score2[k];
                    meter[k]=meter2[k];
                  }
			         for(k=0;name2[k]!=0;k++)
                   name[k]=name2[k];
                  name[k]=0;
			       }
			       else
                 goto EXIT;
			       break;
	        case 1:
	             if(id<tolnum)
			       {
	               TR0=0;TR1=0;
			         id++;
			         for(k=0;k<8;k++)
                   if(ReadOne(k<<4)==tolnum-id+1)
				       {
				         len=ReadOne((k<<4)+1);
				         t=0;
				         while((name[t]=ReadOne((k<<4)+2+t))!=0)
				          t++;
				         Read24C16(((uint)k<<5)+0x100,len,score);
				         Read24C16(((uint)k<<5)+0x200,len,meter);
				       }
			        }
			        else
                  goto EXIT;
			        break;
           default:
                 goto EXIT;
         }
  
         music_disp();
         TH0=Octavec1[score[0]];TL0=TH0;
         count1=0;count2=0;i=0;
         TL1=0;TH1=0;
         TR0=1;TR1=1;
			break;
	 case 3:
         switch(status)
         {
           case 0:
	             if(id>1)
                {
			         TR1=0;TR0=0;
                  id--;
			         len=len1;
			         for(k=0;k<len;k++)
                  {
                    score[k]=score1[k];
                    meter[k]=meter1[k];
                  }
			         for(k=0;name1[k]!=0;k++)
                   name[k]=name1[k];
                  name[k]=0;
			       }
			       else
                 goto EXIT;
			       break;
	        case 1:
	             if(id>1)
			       {
			         TR1=0;TR0=0;
			         id--;
			         for(k=0;k<8;k++)
                   if(ReadOne(k<<4)==tolnum-id+1)
				       {
				         len=ReadOne((k<<4)+1);
				         t=0;
				         while((name[t]=ReadOne((k<<4)+2+t))!=0)
				          t++;
				         Read24C16(((uint)k<<5)+0x100,len,score);
				         Read24C16(((uint)k<<5)+0x200,len,meter);
				       }
			        }
			        else
                  goto EXIT;
			        break;
           default:
                 goto EXIT;
         }
  
         music_disp();
         TH0=Octavec1[score[0]];TL0=TH0;
         count1=0;count2=0;i=0;
         TL1=0;TH1=0;
         TR0=1;TR1=1;
			break;
	 case 4:
         TR1=0;TR0=0;
         LCD_CLEAR();
         k=0;
         while(str_play[k]!=0)
          lcd_wdat(str_play[k++]);
         EX1=1;
         status=2;
			break;
	 case 5:
         TR1=0;TR0=0;
         LCD_CLEAR();
         k=0;
         while(str_record[k]!=0)
          lcd_wdat(str_record[k++]);
         EX1=1;
         TH1=0;TL1=0;count2=0;i=0;
         status=3;
         TR1=1;
			break;
	 case 6:
         t=3;
         if(status!=3)
          goto EXIT;
         TR1=0;TR0=0;
         EX1=0;

         tolnum=0;
         for(k=0;k<8;k++)
          if(ReadOne(k<<4)!=0)
	        tolnum++;
         if(tolnum>=8)
         {
           LCD_CLEAR();
           k=0;
	        while(str_memf[k]!=0)
	         lcd_wdat(str_memf[k++]);
	        goto EXIT;
         }

         LCD_CLEAR();
         k=0;
         while(str_input[k]!=0)
         {
			  lcd_wdat(str_input[k++]);
			  	 if(k>0x0F)
	           lcd_pos(0x30+k);
         }
         while(SHIFT && BACK && FORE && NEXT && OK);
			while(OK==0);
         LCD_CLEAR();
         LCD_INPUT_START();
         k=0;
         while(OK!=0)
         {
           if(SHIFT==0)
           {
	          t=t>2?0:t+1;
             name[k]=(t==0?65:(t==1?97:48));
		       lcd_pos(k>0x0F?k+0x30:k);
		       lcd_wdat(name[k]);
				 LCD_INPUT_FLASH();
				 while(SHIFT==0);
	        }
	        if(BACK==0)
	        {
	          name[k]--;
	          if(name[k]==31)
		        name[k]=(t==0?90:(t==1?122:57));
		       if(name[k]==64||name[k]==96||name[k]==47)
		        name[k]=32;
		       lcd_pos(k>0x0F?k+0x30:k);
		       lcd_wdat(name[k]);
				 LCD_INPUT_FLASH();
				 while(BACK==0);
	        }
	        if(FORE==0)
	        {
	          name[k]++;
	          if(name[k]==33)
		        name[k]=(t==0?'A':(t==1?'a':'0'));
		       if(name[k]==91||name[k]==123||name[k]==58)
		        name[k]=32;
		       lcd_pos(k>0x0F?k+0x30:k);
		       lcd_wdat(name[k]);
				 LCD_INPUT_FLASH();
				 while(FORE==0);
	        }
	        if(NEXT==0)
	        {
	          k=k<8?k+1:k;
		       name[k]=(t==0?65:(t==1?97:48));
		       lcd_pos(k>0x0F?k+0x30:k);
		       lcd_wdat(name[k]);
				 LCD_INPUT_FLASH();
				 while(NEXT==0);
	        }
         }
         name[k+1]=0;
         LCD_INPUT_OK();
         for(k=0;k<8;k++)
          if(ReadOne(k<<4)==0)
          {
			   uchar m;
            t=3;
	         while(~PageWrite24C16(((uint)k<<5)+0x100,0x10,score)&&t>0)
	          t--;
	         if(t<=0)
		       break;
	         t=3;
	         while(~PageWrite24C16(((uint)k<<5)+0x200,0x10,meter)&&t>0)
	          t--;
	         if(t<=0)
		       break;
				t=3;
	         while(~PageWrite24C16(((uint)k<<5)+0x110,0x10,score+16)&&t>0)
	          t--;
	         if(t<=0)
		       break;
	         t=3;
	         while(~PageWrite24C16(((uint)k<<5)+0x210,0x10,meter+16)&&t>0)
	          t--;
	         if(t<=0)
		       break;
				for(m=0;name[m]!=0;m++)
				{
	           t=3;
	           while(~WriteOne((uint)(k<<4)+2+m,name[m])&&t>0)
	            t--;
	           if(t<=0)
		         break;
				}
	         t=3;
	         while(~WriteOne((uint)(k<<4)+2+m,0)&&t>0)
	          t--;
	         if(t<=0)
		       break;
	         t=3;
	         while(~WriteOne((k<<4)+1,i)&&t>0)
	          t--;
	         if(t<=0)
		       break;
	         t=3;
	         while(~WriteOne(k<<4,tolnum+1)&&t>0)
	          t--;
	         if(t<=0)
		       break;
	         break;
	       }
         LCD_CLEAR();
         if(t<=0)
         {
           k=0;
           while(str_error[k]!=0)
            lcd_wdat(str_error[k++]);
         }
         else
         {
           k=0;
           while(str_save[k]!=0)
            lcd_wdat(str_save[k++]);
         }
         status=4;
			break;
	 case 7:
         if(status!=1)
          goto EXIT;
         TR1=0;TR0=0;
         for(k=0;k<8;k++)
         {
	        t=ReadOne(k<<4);
           if(t==tolnum-id+1)
	         if(~WriteOne(k<<4,0))
	          {k=0;break;}
	        if(t>tolnum-id+1)
	         if(~WriteOne(k<<4,t-1))
	          {k=0;break;}
         }
         if(k==0)
         {
           EA=0;
	        LCD_CLEAR();
	        k=0;
	        while(str_delf[k]!=0)
            lcd_wdat(str_delf[k++]);
           break;
         }

         tolnum--;
			id=id>tolnum?id-1:id;
			LCD_CLEAR();
         k=0;
         while(str_dels[k]!=0)
          lcd_wdat(str_dels[k++]);
			time(2000);
			if(tolnum==0)
			{
			  LCD_CLEAR();
			  k=0;
           while(str_nor[k]!=0)
            lcd_wdat(str_nor[k++]);
			  status=5;
			  break;
			}
         for(k=0;k<8;k++)
          if(ReadOne(k<<4)==tolnum-id+1)
	       {
	         len=ReadOne((k<<4)+1);
	         t=0;
            while((name[t]=ReadOne((k<<4)+2+t))!=0)
	          t++;
	         Read24C16(((uint)k<<5)+0x100,len,score);
            Read24C16(((uint)k<<5)+0x200,len,meter);
	       }

         music_disp();
         TH0=Octavec1[score[0]];TL0=TH0;
         count1=0;count2=0;i=0;
         TL1=0;TH1=0;
         TR0=1;TR1=1;
			break;
	}
EXIT:
	IE0=0;
}

void KEYBOARD(void) interrupt 2
{
  uchar key,k;

  if(status==3)
  {
    TR1=0;
    if(i>23)
	 {
	   LCD_CLEAR();
		k=0;
		while(str_overflow[k]!=0)
	   {
		  lcd_wdat(str_overflow[k++]);
		  if(k>0x0F)
	      lcd_pos(0x30+k);
      }
		return;
	 }
	 if(count2>3)
	 {
	   score[i]=0;
	   meter[i]=count2;
	   i++;
	 }
	 TH1=0;TL1=0;count2=0;
	 TR1=1;
  }
	 
  key=((KEY>>3)<<1)+1;
  key=key>6?(key>14?key-2:key-1):key;
  TH0=Octavec1[key];TL0=TH0;
  count1=0;
  TR0=1;
  while(INT1==0);
  TR0=0;
  if(status==3)
  {
    TR1=0;
    if(i>23)
	 {
	   LCD_CLEAR();
		k=0;
		while(str_overflow[k]!=0)
	   {
		  lcd_wdat(str_overflow[k++]);
		  if(k>0x0F)
	      lcd_pos(0x30+k);
      }
		return;
	 }
	 if(count2>0)
	 {
	   score[i]=key;
	   meter[i]=count2;
	   i++;
	 }
	 TH1=0;TL1=0;count2=0;
	 TR1=1;
  }
}

void music_disp()
{
  uchar k;
  LCD_CLEAR();
  lcd_wdat(id+48);
  lcd_wdat('/');
  lcd_wdat(tolnum+48);
  lcd_wdat(' ');
  k=0;
  while(name[k]!=0)
  {
    lcd_wdat(name[k++]);
	 if(k>0x0B)
	  lcd_pos(0x30+k);
  }
}
