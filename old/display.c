#include "display.h"
#include "all.h"

/*void Display(&RTC rtc
{
  
}*/

void ShowDigit(unsigned char numL, unsigned char numR, unsigned char n)
{
  if(n==1){
  PORTD_Bit1 = 1; PORTD_Bit0 = 0; PORTA_Bit7 = 0;
  SetLNum(numL); SetRNum(numR);
  __delay_cycles(10000); 
  //SetLNum(10); SetRNum(10); PORTD_Bit1 = 0; PORTD_Bit0 = 0; PORTA_Bit7 = 0;
  }
  if(n==2){
  PORTD_Bit1 = 0; PORTD_Bit0 = 1; PORTA_Bit7 = 0;
  SetLNum(numL); SetRNum(numR);
  __delay_cycles(10000);
  //SetLNum(10); SetRNum(10); PORTD_Bit1 = 0; PORTD_Bit0 = 0; PORTA_Bit7 = 0;
  }
  if(n==3){
  PORTD_Bit1 = 0; PORTD_Bit0 = 0; PORTA_Bit7 = 1;
  SetLNum(numL); SetRNum(numR);
  __delay_cycles(10000);
  //SetLNum(10); SetRNum(10); PORTD_Bit1 = 0; PORTD_Bit0 = 0; PORTA_Bit7 = 0;
  }
}

void SetLNum(unsigned char A)
{
  switch(A){
case 0: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=1; PORTC_Bit5=0; break;
case 1: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=1; PORTC_Bit5=1; break;
case 2: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=1; break;
case 3: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=0; PORTC_Bit5=1; break;
case 4: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=0; break;
case 5: PORTC_Bit4=1; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=0; break;
case 6: PORTC_Bit4=1; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=1; break;
case 7: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=1; PORTC_Bit5=0; break;
case 8: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=0; PORTC_Bit5=0; break;
case 9: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=1; PORTC_Bit5=1; break;
case 10: PORTC_Bit4=1; PORTC_Bit2=0; PORTC_Bit3=1; PORTC_Bit5=0; break;
  }
}

void SetRNum(unsigned char B)
{
  switch(B){
case 0: PORTA_Bit1=1; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=1; break;
case 1: PORTA_Bit1=1; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=0; break;
case 2: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=1; PORTA_Bit0=1; break;
case 3: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=0; PORTA_Bit0=1; break;
case 4: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=1; PORTA_Bit0=0; break;
case 5: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=1; PORTA_Bit0=1; break;
case 6: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=1; PORTA_Bit0=0; break;
case 7: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=0; break;
case 8: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=0; PORTA_Bit0=0; break;
case 9: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=1; break;
case 10: PORTA_Bit1=1; PORTA_Bit3=0; PORTA_Bit2=1; PORTA_Bit0=0; break;
  }
}
