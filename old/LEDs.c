#include "all.h"

void LEDs(LED *p)
{
  static u8 counterLed=0;
  switch(counterLed){
    case 0:
      //OCR1A=0; OCR1B=0; OCR2=0;
       PORTD_Bit3=1; PORTD_Bit6=0; PORTC_Bit7=0; 
       PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=0;
       //OCR1A = p->R[0]; OCR2 = p->G[0]; OCR1B = p->B[0];
       OCR1A = 0; OCR2 = 0; OCR1B = 0;
       __delay_cycles(50);
       break;
    case 1:
      //OCR1A=0; OCR1B=0; OCR2=0;
       PORTD_Bit3=0; PORTD_Bit6=1; PORTC_Bit7=0; 
       PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=0;
       //OCR1A = p->R[1]; OCR2 = p->G[1]; OCR1B = p->B[1];
       OCR1A = 255; OCR2 = 255; OCR1B = 255;
       __delay_cycles(50); 
       break;
    case 2:
      //OCR1A=0; OCR1B=0; OCR2=0;
       PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=1; 
       PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=0;
       //OCR1A = p->R[2]; OCR2 = p->G[2]; OCR1B = p->B[2];
       OCR1A = 0; OCR2 = 0; OCR1B = 0;       
       __delay_cycles(50); 
       break;
    case 3:
      //OCR1A=0; OCR1B=0; OCR2=0;
       PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=0; 
       PORTA_Bit6=1; PORTB_Bit0=0; PORTB_Bit1=0;
       //OCR1A = p->R[3]; OCR2 = p->G[3]; OCR1B = p->B[3];
       OCR1A = 255; OCR2 = 255; OCR1B = 255;
       __delay_cycles(50);
       break;
    case 4:
      //OCR1A=0; OCR1B=0; OCR2=0;
       PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=0; 
       PORTA_Bit6=0; PORTB_Bit0=1; PORTB_Bit1=0;
       //OCR1A = p->R[4]; OCR2 = p->G[4]; OCR1B = p->B[4];
       OCR1A = 0; OCR2 = 0; OCR1B = 0;
       __delay_cycles(50); 
       break;
    case 5:
      //OCR1A=0; OCR1B=0; OCR2=0;
       PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=0; 
       PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=1;
       //OCR1A = p->R[5]; OCR2 = p->G[5]; OCR1B = p->B[5];
       OCR1A = 255; OCR2 = 255; OCR1B = 255;
       __delay_cycles(50); 
       break;
  }
  if (counterLed==2) counterLed=0;
  else counterLed++;
}
