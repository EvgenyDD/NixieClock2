/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>

#include "all.h"
#include "ds1307.h"
#include "backlight.h"
#include "transient.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
enum {BTN_SET, BTN_MINUS, BTN_PLUS};

__flash uint8_t DayBright[25] = {1,21,42,63,84,105,126,147,168,189,
                210,231,245,231,210,189,168,147,126,105,84,63,42,21,1}; 
__flash uint8_t NightBright[25] = {105,116,127,138,149,162,175,186,198,210,
                222,234,245,234,222,210,198,186,175,162,149,138,127,116,105}; 

__flash uint8_t DC[4][22] = 
{
    {0,0,0,0,0,1,1,0,0,0,1, 1,1,0,0,0,0,0,0,0,0,1},
    {0,0,0,1,0,0,0,1,1,1,0, 0,0,1,1,1,0,0,0,1,0,0},
    {1,1,0,0,0,0,0,1,0,1,1, 0,0,1,0,1,1,1,0,0,0,1},
    {0,1,1,1,0,0,1,0,0,1,0, 1,0,1,1,0,1,0,0,0,1,0}
};


/* Private macro -------------------------------------------------------------*/
#define DOTS_ON     PORTA_Bit5 = PORTC_Bit6 = 1;  
#define DOTS_OFF    PORTA_Bit5 = PORTC_Bit6 = 0; 

#define BUTTON_RESET(btn, debounce)     protectKeys[(btn)] = (debounce);\
                                        BitReset(button, (btn));


/* Private variables ---------------------------------------------------------*/
__eeprom __no_init uint8_t eLEADZERO;   
__eeprom __no_init uint8_t eSEPARATOR;  
__eeprom __no_init uint8_t eMIX_DELAY;  
__eeprom __no_init uint8_t eBL_MODE;    

volatile uint8_t settingsMode = 0, settingsFlag=0;  //setting mode active flag
volatile uint8_t Global1HzFlag = 0;
volatile uint8_t outR[6], outG[6], outB[6]; //output BL values
    
volatile uint8_t button;                    //buttons set flags  
volatile uint8_t protectKeys[3]={0,0,0};    //button debounce

volatile uint8_t counter = 0;             
volatile uint8_t digit = 0;

uint8_t hour, min, sec,  day, mon, year; 

//tube output values
volatile uint8_t out[6] = {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK};  
uint8_t targetOut[6];              //new output number

volatile uint8_t changedDigit;  //flag->the digit is changed    
volatile uint8_t tranCounter=0; //counter to set bright levelel from Bright massives

uint8_t level[6] = {1};  



/* Extern variables ----------------------------------------------------------*/
extern struct SmoothType smooth[SMOOTH_NUM_CHANNELS];
extern uint8_t BLType;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : RandomGet
* Description    : Generate 4-digit random number in range [400;600]
*******************************************************************************/
uint16_t RandomGet()
{
    uint16_t temp = ADCL;
    temp |= (ADCH << 8);
    ADCSRA |= (1<<ADSC); //enable ADC interrupt
    
    return temp+rand(); 
}


/*******************************************************************************
* Function Name  : RandomGet4
* Description    : Generate 4-digit random number
* Return         : 4-digit random number
*******************************************************************************/
uint16_t RandomGet4()
{
    uint16_t a = ADCL;
    a |= (ADCH << 8);
    ADCSRA |= (1<<ADSC); //enable ADC interrupt
    while(BitIsReset(ADCSRA, ADIF));
    
    uint16_t b = ADCL;
    b |= (ADCH << 8);
    ADCSRA |= (1<<ADSC); //enable ADC interrupt
    while(BitIsReset(ADCSRA, ADIF));
    
    uint16_t c = ADCL;
    c |= (ADCH << 8);
    ADCSRA |= (1<<ADSC); //enable ADC interrupt
    while(BitIsReset(ADCSRA, ADIF));
    
    uint16_t D = ADCL;
    D |= (ADCH << 8);
    ADCSRA |= (1<<ADSC); //enable ADC interrupt
    while(BitIsReset(ADCSRA, ADIF));
    
    return (uint16_t)(a*1000+b*100+c*10+D+rand()); 
}


/*******************************************************************************
* Function Name  : Initialization
* Description    : Initialize thw peripheral
*******************************************************************************/
void Initialization()
{
//I/O init
    DDRA = 0xEF; 
    DDRB = 0x0B;  
    DDRC = 0xFF;  
    DDRD = 0x7B;

    PORTD = 0x04;
    
//ADC Init for Random - PA4
    ADMUX = (1<<REFS0)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0);               
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);       
    SFIOR = 0;    
       
//Timers 1,2 for PWM
    TCCR0  |= (1<<WGM00)|(1<<WGM01)|(0<<COM00)|(1<<COM01)|(1<<CS00);
    TCCR1A |= (1<<COM1A1)|(1<<COM1B1)|(0<<COM1A0)|(0<<COM1B0)|(1<<WGM10)|(0<<WGM11);
    TCCR1B |= (1<<WGM12)|(0<<WGM13)|(0<<CS12)|(0<<CS11)|(1<<CS10);   
       
//Timer 2 for interrupts
    TCCR2 |= (1<<CS20)|(1<<CS21);
    TIMSK |= (1<<OCIE2)|(1<<TOIE2);    
    
    MCUCR = (1<<ISC01) | (1<<ISC00);   
    GICR = (1<<INT0);           
    
//RTC Init
    DS1307Init();    
  
    srand(RandomGet4());
      
    if(eLEADZERO > 1) eLEADZERO = 0;
    if(eSEPARATOR > 1) eSEPARATOR = 0;
    if(eMIX_DELAY > 99) eMIX_DELAY = 10;  
    if(eBL_MODE > 5) eBL_MODE = 0;  
    BLType = eBL_MODE;
  
//Check clock
    DS1307GetDate(&day, &mon, &year); 
    DS1307GetTime(&hour, &min, &sec);  
    
    uint8_t tempFlag=0;
    if(day>31) {day = 1; tempFlag++;}
    if(mon>12) {mon = 1; tempFlag++;}
    if(year>99) {year = 0; tempFlag++;}
    if(tempFlag) DS1307SetDate(day, mon, year);
    tempFlag = 0;
    
    if(hour>24) {hour = 0; tempFlag++;} 
    if(min>59) {min = 0; tempFlag++;}
    if(sec>59) {sec = 0; tempFlag++;}
    if(tempFlag) DS1307SetTime(hour, min, sec);
  
    ZeroBL();        
    _delay_ms(200);
    
    __enable_interrupt();
}


/*******************************************************************************
* Function Name  : GetDate.
* Description    : Get date from DS1307 and display it 1.5 sec
*******************************************************************************/
void GetDate()
{    
    DOTS_OFF;    
    ZeroBL();
    
    DS1307GetDate(&day, &mon, &year);
    
    for(uint8_t i=0; i<6; i++) level[i] = 10;   
    Sprint(day, mon, year); 
    
    _delay_ms(1500);
}


/*******************************************************************************
* Function Name  : Settings
* Description    : Settings Mode Handler
*******************************************************************************/
void Settings()
{ 
    /* settingsMode:
    * 1: hours
    * 2: minutes
    * 3: seconds
    *
    * 4: day
    * 5: month
    * 6: year
    *
    * 7: leading zero
    * 8: dots
    * 9: main backlight type
    * 10: mix mode change time
    */

    if(SQW_IN)
    {
        switch(settingsMode)
        {
            case 1: out[0] = hour/10; 
                    out[1] = hour%10;
                break;
            case 2: out[2] = min/10; 
                    out[3] = min%10;    
                break;
            case 3: out[4] = sec/10; 
                    out[5] = sec%10;
                break;
            case 4: out[0] = day/10; 
                    out[1] = day%10;
                break;  
            case 5: out[2] = mon/10; 
                    out[3] = mon%10; 
                break;
            case 6: out[4] = year/10; 
                    out[5] = year%10; 
                break;
            case 7: out[0] = eLEADZERO;
                break;
            case 8: out[1] = eSEPARATOR; 
                break;
            case 9: out[2] = eBL_MODE/10; 
                    out[3] = eBL_MODE%10;
                break;
            case 10:out[4] = eMIX_DELAY/10; 
                    out[5] = eMIX_DELAY%10;    
                break;
        }  
    }
    else
    {
        switch(settingsMode)
        {
            case 1: out[0] = out[1] = BLANK; 
                break;
            case 2: out[2] = out[3] = BLANK; 
                break;
            case 3: out[4] = out[5] = BLANK;
                break;
            case 4: out[0] = out[1] = BLANK;
                break;  
            case 5: out[2] = out[3] = BLANK;      
                break;
            case 6: out[4] = out[5] = BLANK;
                break;
            case 7: out[0] = BLANK; 
                break;
            case 8: out[1] = BLANK;    
                break;
            case 9: out[2] = out[3] = BLANK; 
                break;
            case 10:out[4] = out[5] = BLANK;
                break;
        }
    }

    
    if(BitIsSet(button, BTN_PLUS))
    {
        BUTTON_RESET(BTN_PLUS, 5);
        
        switch(settingsMode)
        {
            case 1: if(++hour > 23) hour = 0;  
                break;
            case 2: if(++min > 59) min = 0;
                break;
            case 3: sec = 0; 
                break;
            case 4: if(++day > 31) day = 1; 
                break;  
            case 5: if(++mon > 12) mon = 1; 
                break;
            case 6: if(++year > 99) year = 0;
                break;
            case 7: eLEADZERO = 1;
                break;
            case 8: eSEPARATOR = 1; 
                break;
            case 9: if(eBL_MODE < 6) eBL_MODE++; 
                break;
            case 10:if(eMIX_DELAY < 99) eMIX_DELAY++; 
                break;
        }
    }
    
    if(BitIsSet(button, BTN_MINUS))
    {
        BUTTON_RESET(BTN_MINUS, 5);
        
        switch(settingsMode)
        {
            case 1: if(--hour > 23)  hour = 23; 
                break;
            case 2: if(--min > 59)  min = 59;  
                break;
            case 3: sec = 0; 
                break;
            case 4: if(--day < 1) day = 31;  
                break;  
            case 5: if(--mon < 1) mon = 12; 
                break;
            case 6: if(--year > 99) year = 0;
                break;
            case 7: eLEADZERO = 0;
                break;
            case 8: eSEPARATOR = 0; 
                break;
            case 9: if(eBL_MODE > 0) eBL_MODE--; 
                break;
            case 10:if(eMIX_DELAY > 10) eMIX_DELAY--;  
                break;
        }
    }
}




/*******************************************************************************
* Function Name  : Buttons
* Description    : Buttons press handler
*******************************************************************************/
void Buttons()
{ 
    if(BitIsSet(button, BTN_SET) && !settingsMode)
    {  
        BUTTON_RESET(BTN_SET, 5);
        
        BackLightChange();        
    }
          
    if(BitIsSet(button, BTN_SET) && settingsMode)
    {       
        BUTTON_RESET(BTN_SET, 5);
        
        if(settingsMode <= 3) 
            DS1307SetTime(hour, min, sec);  
        
        if(settingsMode > 3 && settingsMode < 7) 
            DS1307SetDate(day, mon, year); 
        
        ZeroBL();
        
        switch(++settingsMode)
        {
            case 1: outB[0]=255; outB[1]=255; break;
            case 2: outG[2]=255; outG[3]=255; break;
            case 3: outR[4]=255; outR[5]=255; break;
            
            case 4: outB[0]=255; outB[1]=255; outR[0]=255; outR[1]=255; break;
            case 5: outG[2]=255; outG[3]=255; outR[2]=255; outR[3]=255; break;
            case 6: outG[4]=255; outG[5]=255; outB[4]=255; outB[5]=255; break;
            
            case 7: outB[0]=255; break;
            case 8: outR[1]=255; outG[1]=255; break;
            case 9: outR[2]=255; outR[3]=255; break;
            case 10:outB[4]=255; outB[5]=255; break;
        }
        
        if(settingsMode <= 3) 
        {
            out[0] = hour/10; 
            out[1] = hour%10;
            out[2] = min/10; 
            out[3] = min%10;
            out[4] = sec/10; 
            out[5] = sec%10;
        }
        
        if(settingsMode > 3 && settingsMode < 7) 
        {
            out[0] = day/10; 
            out[1] = day%10;
            out[2] = mon/10; 
            out[3] = mon%10;
            out[4] = year/10; 
            out[5] = year%10;
        }
        
        if(settingsMode >= 7) 
        {
            out[0] = eLEADZERO; 
            out[1] = eSEPARATOR;
            out[2] = eBL_MODE/10; 
            out[3] = eBL_MODE%10; 
            out[4] = eMIX_DELAY/10;
            out[5] = eMIX_DELAY%10; 
        }
        
        if(settingsMode > 10) settingsMode = 0;    
    }      
    
    
    //Button MINUS
    if(BitIsSet(button, BTN_MINUS) && !settingsMode)
    {  
        BUTTON_RESET(BTN_MINUS, 10);
        
        switch(BLGetType())
        {
            case 0: GetDate(); 
                break;
            case 1: GetDate();  
                break;                           
            case 2: GetDate(); 
                break;     
            case 3: BinaryChangeEffect(DIR_DOWN);
                break;               
            case 4: GetDate(); 
                break;      
            case 5: GetDate();
                break;
            case 6: HSVchange(DIR_DOWN);
                break; 
        }
    }
        
    //Button PLUS
    if(BitIsSet(button, BTN_PLUS) && !settingsMode)
    {  
        BUTTON_RESET(BTN_PLUS, 10);
        
        switch(BLGetType())
        {
            case 0: GetDate(); 
                break;
            case 1: GetDate();  
                break;                           
            case 2: GetDate(); 
                break;     
            case 3: BinaryChangeEffect(DIR_UP);
                break;               
            case 4: GetDate(); 
                break;      
            case 5: GetDate();
                break;
            case 6: HSVchange(DIR_UP);
                break;
        }
    }
}

/*******************************************************************************
* Function Name  : Main
* Description    : Main routine
*******************************************************************************/
void main()
{
    Initialization(); 
    for(uint8_t i=0; i<SMOOTH_NUM_CHANNELS; i++) SmoothInit(i);
    
    while(1)
    {             
        if(settingsFlag)        //correct initialization of settings mode
        {
            settingsMode = 1;
            settingsFlag = 0;
            ZeroBL();
            outB[0]=255; 
            outB[1]=255;
        }
        
        if(Global1HzFlag && !settingsMode)  //time update
        {      
            Global1HzFlag = 0;
            
            DS1307GetTime(&hour, &min, &sec);
            Soft(hour, min, sec);   
            
            if(!BLType) MixBLReduceTime();
        }
        
        if(settingsMode) Settings();            //settings mode
        if(!settingsMode) BackLightProcess();   //backlight processing
        
        Buttons();                              //buttons processing
        
        if(!settingsMode && eSEPARATOR)         //dots
            DOTS_ON 
        else                                
            DOTS_OFF;
    }
}



/*******************************************************************************
* Function Name  : Over
* Description    : Timer2 overflow handler - 2.441 Hz
*******************************************************************************/
#pragma vector = TIMER2_OVF_vect 
__interrupt void Over()
{    
//Digits    
    //DC: OUT disable
    PORTC |=   (1<<3)|(1<<4);
    PORTC &= ~((1<<2)|(1<<5));
    PORTA |=   (1<<1)|(1<<2);
    PORTA &= ~((1<<0)|(1<<3));
 
    OCR2 = level[digit]; //255 = min  1 = max
    
    if (tranCounter < 25) //bright mass has 25 values
    { 
        static uint8_t tranSmooth = 0;           
        
        if (++tranSmooth >= 50)         //bigger -> smoother numbers change
        { 
            tranSmooth = 0;
            
            for (uint8_t i = 0; i < 6; i++) 
            {
                if (BitIsSet(changedDigit, i) || tranCounter == 0)
                {
                    if(hour > 7) 
                        level[i] = DayBright[tranCounter]; 
                    else 
                        level[i] = NightBright[tranCounter];
                }
                
                if (tranCounter == 25/2 && settingsMode == 0)
                {
                    out[i] = targetOut[i];  //on min brightness -> update digit
                }
            }
            
            tranCounter++;
         }
    }                
        
//BackLight: PWM
    static uint8_t pwmCnt = 0;      
    
    if(++pwmCnt > 6) 
    {
        pwmCnt = 0;
        
        PORTD_Bit3 = PORTD_Bit6 = PORTC_Bit7 = PORTA_Bit6 = PORTB_Bit0 = PORTB_Bit1 = 0;  
        switch(counter)
        {
            case 0: PORTD_Bit3=1; break;
            case 1: PORTD_Bit6=1; break;
            case 2: PORTC_Bit7=1; break;
            case 3: PORTA_Bit6=1; break;
            case 4: PORTB_Bit0=1; break;
            case 5: PORTB_Bit1=1; break;  
        }   
        
        OCR1A = outR[counter];
        OCR0  = outG[counter];
        OCR1B = outB[counter];
    }    
        
//Buttons  
    static uint16_t timerButton[3]={0,0,0};   //count++ while button pressed

    //Button #0  
    DDRB_Bit4 = 1; 
    PORTB_Bit4 = 0; 
    DDRB_Bit4 = 0;    
    __delay_cycles(15);
    
    if(PINB_Bit4 == 0) 
        timerButton[0]++; 
    else
    {
        if(timerButton[0] > 1000)           //settings mode - on long press
        { 
            BitReset(button, 0);
            timerButton[0] = protectKeys[0] = 0;
            
            settingsFlag = 1; 
        }
        
        if(timerButton[0] < 1000 && timerButton[0] > 200)
        { 
            BitSet(button, 0);
            timerButton[0] = protectKeys[0] = 0;
        }
    }   
        
    //Button #1  
    DDRD_Bit7 = 1; 
    PORTD_Bit7 = 0; 
    DDRD_Bit7 = 0;     
    __delay_cycles(15);
    
    if(BitIsReset(PIND, 7))      
        timerButton[1]++;    
    else                    
    {
        timerButton[1] = protectKeys[1] = 0;
    }
    
    if(timerButton[1] == 150)   //240 - 100ms button non-reaction
    {
        BitSet(button, 1); 
        timerButton[1] = 0;
    } 
    
    //Button #2  
    DDRB_Bit2 = 1; 
    PORTB_Bit2 = 0; 
    DDRB_Bit2 = 0;    
    __delay_cycles(15);
    
    if(BitIsReset(PINB, 2))        
        timerButton[2]++;    
    else                    
    {
        timerButton[2] = protectKeys[2] = 0; 
    }
    
    if(timerButton[2] == 150)   //240 - 100ms button non-reaction
    {
        BitSet(button, 2); 
        timerButton[2] = 0;
    } 
   
    
    //debounce implementation
    for(uint8_t i=0; i<3; i++)
    {
        //protectKeys=1 -> debounce 100ms
        if(protectKeys[i]) BitReset(button, i);
    }
    

    static uint8_t counterBtn = 0;    
    if(++counterBtn >= 240)                 
    {
        counterBtn = 0;
        
        if(protectKeys[0]) protectKeys[0]--;
        if(protectKeys[1]) protectKeys[1]--;
        if(protectKeys[2]) protectKeys[2]--;
    }    

//BackLight: wait time counters    
    for(uint8_t i=0; i<18; i++)
    {
        if(smooth[i].waitTimer)  smooth[i].waitTimer--;
    }
}




/*******************************************************************************
* Function Name  : Compare
* Description    : Timer2 compare handler: PWM digits
*******************************************************************************/
#pragma vector = TIMER2_COMP_vect 
__interrupt void Compare() // реализация ШИМ управления яркостью ламп и подсветки
{  
//Anode commutations
    if(counter==0 || counter==3) 
    {
        PORTD_Bit1 = 1; 
        PORTD_Bit0 = 0; 
        PORTA_Bit7 = 0;
    }
    
    if(counter==1 || counter==4) 
    {
        PORTD_Bit1 = 0; 
        PORTD_Bit0 = 1; 
        PORTA_Bit7 = 0;
    }
    
    if(counter==2 || counter==5) 
    {
        PORTD_Bit1 = 0; 
        PORTD_Bit0 = 0; 
        PORTA_Bit7 = 1;
    }

//DC: left
    if(digit < 3)//левая половина ламп - дешифратор 
    {
        PORTC_Bit4 = DC[0][ out[digit] ]; 
        PORTC_Bit2 = DC[1][ out[digit] ]; 
        PORTC_Bit3 = DC[2][ out[digit] ]; 
        PORTC_Bit5 = DC[3][ out[digit] ];
    }
//DC: right
    else
    {
        PORTA_Bit1 = DC[0][ 11+out[digit] ]; 
        PORTA_Bit3 = DC[1][ 11+out[digit] ]; 
        PORTA_Bit2 = DC[2][ 11+out[digit] ]; 
        PORTA_Bit0 = DC[3][ 11+out[digit] ];
    }              

//Digit counters
    if(++counter == 6) counter = 0;
    if(counter <  3) digit = counter;
    if(counter >= 3) digit = 8 - counter;
}



/*******************************************************************************
* Function Name  : Update
* Description    : INT0 handler: 1Hz update command from DS1307
*******************************************************************************/
#pragma vector = INT0_vect 
__interrupt void Update()
{ 
    Global1HzFlag = 1;
}