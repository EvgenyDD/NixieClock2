/* Includes ------------------------------------------------------------------*/
#include "all.h"
#include "ds1307.h"
#include "transient.h"
#include "backlight.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern __eeprom __no_init uint8_t eLEADZERO;
extern volatile uint8_t settingsMode;

extern volatile uint8_t out[6];
extern  uint8_t targetOut[6];

extern volatile uint8_t tranCounter;
extern volatile uint8_t changedDigit;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Soft
* Description    : Smooth update of nubmers
* Input          : Three numbers to display next
*******************************************************************************/
void Soft(uint8_t Num1, uint8_t Num2, uint8_t Num3) 
{     
    if(!settingsMode)
    {
        if(!eLEADZERO && Num1<10)   targetOut[0] = 10;
        else                        targetOut[0] = Num1/10; 
        
        targetOut[1] = Num1 % 10;    
        targetOut[2] = Num2 / 10;
        targetOut[3] = Num2 % 10;    
        targetOut[4] = Num3 / 10;
        targetOut[5] = Num3 % 10;
        
        for(uint8_t i=0; i<6; i++) 
        {
            if (targetOut[i] != out[i]) BitSet(changedDigit, i);
            else                        BitReset(changedDigit, i);
        }
        
        tranCounter = 0;
    }
    else
        changedDigit = 0; 
}
  


/*******************************************************************************
* Function Name  : Sprint
* Description    : Digits sprint effect
* Input          : Three numbers to display
*******************************************************************************/
void Sprint(uint8_t Num1, uint8_t Num2, uint8_t Num3)  
{
    ZeroBL();
    
    targetOut[0] = Num1 / 10;    
    targetOut[1] = Num1 % 10;
    targetOut[2] = Num2 / 10;    
    targetOut[3] = Num2 % 10;
    targetOut[4] = Num3 / 10;    
    targetOut[5] = Num3 % 10;

    for(uint8_t i=0; i<6; i++) 
    {
        uint8_t UPDOWN[6];
        
        if (targetOut[i] > out[i]) {UPDOWN[i] = 2; BitSet(changedDigit, i);} // новая больше
        if (targetOut[i] < out[i]) {UPDOWN[i] = 3; BitSet(changedDigit, i);} // новая меньше
        
        for(uint8_t j=0; j<9; j++)
        {
            if(UPDOWN[i] == 2) 
            {
                if(out[i] != targetOut[i]) out[i]++;   // новая цифра больше, плюсуем
            }
            if(UPDOWN[i] == 3) 
            {
                if(out[i] != targetOut[i]) out[i]--;   // новая цифра меньше, вычитаем
            }
            
            if(out[i] == targetOut[i]) break;          // новая цифра равна, возвращаемся в начало
        
            _delay_ms(50);
        }
    }
}


/*******************************************************************************
* Function Name  : ShiftLeft
* Description    : Digits shift to left
* Input          : Number to display
*******************************************************************************/  
void ShiftLeft(uint16_t number) 
{  
    ZeroBL();
    
    for(uint8_t i=5; i>0; i--)   
    {
        __asm("cli");
        out[0] = out[1];    
        out[1] = out[2];
        out[2] = out[3];    
        out[3] = out[4];
        out[4] = out[5];    
        out[i] = BLANK;
        __asm("sei");
        
        _delay_ms(50);
    }
	
    out[0] = BLANK;

    for(uint8_t i=0; i<=3; i++)  
    {
        __asm("cli");
        out[5-i] = number/1000;   
        out[6-i] = number/100%10;
        out[7-i] = number/10%10;    
        out[8-i] = number%10;
        __asm("sei");
        
        _delay_ms(50);      
    }
        
    _delay_ms(200);         
}

 
/*******************************************************************************
* Function Name  : ShiftRight
* Description    : Digits shift to right 
*******************************************************************************/  
void ShiftRight() 
{
    ZeroBL();   
    
    if(!out[0]) 
    {
        //positive
        for(uint8_t i = 0; i <= 5; i++) 
        { 
            out[5] = out[4];    
            out[4] = out[3];
            out[3] = out[2];    
            out[2] = out[1];
            out[1] = out[0];    
            out[i] = BLANK;
            
            _delay_ms(50);
        }
    }
    else
    {
        //negative
        for(uint8_t i = 0; i <= 3; i++) 
        {
            out[5] = out[4];    
            out[4] = out[3];
            out[3] = out[2];    
            out[2] = out[1];
            out[i] = BLANK;
            
            _delay_ms(50);
        }
    }

    uint8_t H, M, S;
    DS1307GetTime(&H, &M, &S); 

    targetOut[0] = H / 10;   
    targetOut[1] = H % 10;
    targetOut[2] = M / 10;   
    targetOut[3] = M % 10;
    targetOut[4] = S / 10;   
    targetOut[5] = S % 10;

    for(uint8_t i = 0; i <= 5; i++) 
    { 
        out[5] = out[4];    
        out[4] = out[3];
        out[3] = out[2];    
        out[2] = out[1];
        out[1] = out[0];    
        out[0] = targetOut[5-i];
        
        _delay_ms(50);
    }
}