/* Includes ------------------------------------------------------------------*/
#include "all.h"
#include "backlight.h"
#include "transient.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SMOOTH_PWM_MAX  255
#define NUL 0

enum {WAITING, UP, DOWN};


/* Private macro -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
__flash uint8_t TABLE[][18] = {
{ 1,0,0, 1,0,0, 1,0,0, 1,0,1, 1,0,1, 1,1,0}, //FIRST COLOR
{ 0,1,0, 0,1,0, 0,1,0, 1,1,0, 1,1,1, 0,0,1}, //FIRST COLOR
{ 0,0,1, 0,0,1, 0,0,1, 0,1,1, 0,1,0, 1,1,1}, //FIRST COLOR
{ 0,1,1, 0,0,0, 0,1,1, 0,1,0, 0,1,1, 1,0,1}, //SECOND COLOR
{ 1,0,0, 0,0,1, 1,0,1, 0,0,1, 1,1,0, 1,1,0}, //SECOND COLOR
{ 0,0,0, 1,1,0, 1,1,0, 1,0,0, 1,0,1, 0,1,1}  //SECOND COLOR
}; //binary colors - 2bit color))

__flash uint8_t RandomChannelEachBL[18] = {
 0,17,5, 12,6,11, 1,7,14, 13,16,4, 2,8,10, 9,15,3
}; //make choise more random


/* Private variables ---------------------------------------------------------*/
uint8_t ManualR, ManualG, ManualB;                  //static colors for manual color set
uint8_t binaryType = 0;                             //binary colors set
uint8_t BLType = 0;                                 //backlight type
uint8_t mixTime = 0;                                 //backlight type

struct SmoothType smooth[SMOOTH_NUM_CHANNELS];

struct SmoothSettings sFire = { 
    &GiveTargetBLFire, 
    &GiveSpeedBLFire, 
    &GiveWaitBLFire
};
struct SmoothSettings sRandomAll = {
    &GiveTargetBLRandAll,
    &GiveSpeedBLRandAll,
    &GiveWaitBLRandAll
};
struct SmoothSettings sRandomEach = {
    &GiveTargetBLRandEach,                               
    &GiveSpeedBLRandEach, 
    &GiveWaitBLRandEach
};
struct SmoothSettings sShift = {
    &GiveTargetBLShift,
    &GiveSpeedBLShift,
    &GiveWaitBLShift
};
struct SmoothSettings sBinary = {
    &GiveTargetBLBinary,
    &GiveSpeedBLBinary,
    &GiveWaitBLBinary
};


/* Extern variables ----------------------------------------------------------*/
extern uint8_t sec;
extern volatile uint8_t outR[6], outG[6], outB[6]; 
extern __eeprom __no_init uint8_t eMIX_DELAY;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : ZeroBL
* Description    : Reset all backlight to zero
*******************************************************************************/
void ZeroBL()
{
    for(uint8_t i=0; i<6; i++)
        outR[i] = outG[i] = outB[i] = 0;
}


/*******************************************************************************
* Function Name  : HSVtoRGB
* Description    : Convert HUE -> RGB colors
* Input          : Pointers for Red, Green, Blue
* Input          : Hue value
*******************************************************************************/
void HSVtoRGB( uint8_t *r, uint8_t *g, uint8_t *b, uint16_t i_hue )
{
    uint16_t ii;
    double fr, hue;
    uint8_t c2, c3;
    
    while (i_hue >= 360) i_hue -= 360;
    
    hue = i_hue;
    ii = (uint16_t)(hue /= 60.0);
    fr = hue - ii;
    c2 = (uint8_t)(255 - 255 * fr);
    c3 = (uint8_t)(255 * fr);
    
    switch (ii)
    { 
        case 0: *r = 255; *g = c3;  *b = 0;   break;
        case 1: *r = c2;  *g = 255; *b = 0;   break;
        case 2: *r = 0;   *g = 255; *b = c3;  break;
        case 3: *r = 0;   *g = c2;  *b = 255; break;
        case 4: *r = c3;  *g = 0;   *b = 255; break;
        case 5: *r = 255; *g = 0;   *b = c2;  break;
    }
}


/*******************************************************************************
* Function Name  : SmoothInit
* Description    : Initialize smooth channel
*******************************************************************************/
void SmoothInit(uint8_t channel)
{
    smooth[channel].speed = 1;
    smooth[channel].state = WAITING;
    smooth[channel].current = smooth[channel].target = 0;    
}


/*******************************************************************************
* Function Name  : SmoothProcess
* Description    : Process smooth backlight change
*******************************************************************************/
void SmoothProcess(uint8_t channel, struct SmoothSettings *pSettings)
{
    if(smooth[channel].waitTimer == 0)
    {
        if(smooth[channel].state == WAITING)
        {           
            smooth[channel].target = pSettings->pGetTarget(channel);
            smooth[channel].speed  = pSettings->pGetSpeed(channel);
            
            if(smooth[channel].target > smooth[channel].current)
            {
                smooth[channel].state = UP;
            }
            else
            {
                smooth[channel].state = DOWN;
            }
        }
        
        if(smooth[channel].state == UP) //current is smaller than target
        {
            if((uint16_t)(smooth[channel].current +
                          smooth[channel].speed) <= SMOOTH_PWM_MAX)  
                smooth[channel].current += smooth[channel].speed;
            else                            
                smooth[channel].current = 255;
            
            
            if(smooth[channel].current >= smooth[channel].target) 
                smooth[channel].state = WAITING;
        }
        
        if(smooth[channel].state == DOWN) //current is bigger than target
        {
            if(smooth[channel].current > smooth[channel].speed) 
                smooth[channel].current -= smooth[channel].speed;
            else 
                smooth[channel].current = 0;
            
            
            if(smooth[channel].current <= smooth[channel].target) 
                smooth[channel].state = WAITING;   
        }
        
        //set waiting time
        smooth[channel].waitTimer = pSettings->pGetWaitTime(channel);   
    }
}


/*******************************************************************************
* Function Name  : GetSmoothCurrent
* Description    : Return current bright value
* Input          : Channel
* Return         : Current bright value
*******************************************************************************/
uint8_t GetSmoothCurrent(uint8_t channel)
{
    return smooth[channel].current; 
}




/*============================================================================*/
/*====================== FIRE MODE ===========================================*/
/*============================================================================*/
/*******************************************************************************
* Function Name  : FireBL
* Description    : Fire mode backlight
*******************************************************************************/
void FireBL()	
{    
    for(uint8_t i=0; i<6; i++)
    {
        if(smooth[i].waitTimer == 0)
        {
            outG[i] = outR[i] / (RandomGet4()%25+8);
            outB[i] = 0;
        }
        
        SmoothProcess(i, &sFire);
        outR[i] = GetSmoothCurrent(i);
    }    
}

/*******************************************************************************
* Function Name  : GiveTargetBLFire
* Input          : Channel
* Return         : Target value
*******************************************************************************/
uint8_t GiveTargetBLFire(uint8_t channel) //set color for Fire Mode
{
    return RandomGet4()%230+25;
}

/*******************************************************************************
* Function Name  : GiveSpeedBLFire
* Input          : Channel
* Return         : Speed value
*******************************************************************************/
uint8_t GiveSpeedBLFire(uint8_t channel) //set speed for Fire Mode
{
    return RandomGet()%3+1;
}

/*******************************************************************************
* Function Name  : GiveWaitBLFire
* Input          : Channel
* Return         : Wait time value
*******************************************************************************/
uint16_t GiveWaitBLFire(uint8_t channel)
{
    if(smooth[channel].current< 120 && smooth[channel].state == DOWN)  
        return RandomGet()%10+4;
    else                       
        return RandomGet()%7+2;  
}   




/*============================================================================*/
/*====================== Random - All have one color =========================*/
/*============================================================================*/
/*******************************************************************************
* Function Name  : RandomAllBL
* Description    : 
*******************************************************************************/
void RandomAllBL()
{
    for(uint8_t i=0; i<3; i++)
        SmoothProcess(i, &sRandomAll);
    
    for(uint8_t i=0; i<6; i++)
    {
        outR[i] = GetSmoothCurrent(0);
        outG[i] = GetSmoothCurrent(1);
        outB[i] = GetSmoothCurrent(2);
    }
}

/*******************************************************************************
* Function Name  : GiveTargetBLRandAll
* Input          : Channel
* Return         : Target value
*******************************************************************************/
uint8_t GiveTargetBLRandAll(uint8_t channel)
{
    static uint8_t R=0, G=0, B=0;
    
    if(channel == 0)
    {
        HSVtoRGB(&R, &G, &B , (RandomGet4() % 360));
        return R;
    }
    if(channel == 1) return G;
    if(channel == 2) return B;
    
    return 1;
}

/*******************************************************************************
* Function Name  : GiveSpeedBLRandAll
* Input          : Channel
* Return         : Speed value
*******************************************************************************/
uint8_t GiveSpeedBLRandAll(uint8_t channel)
{
    return 1;
}

/*******************************************************************************
* Function Name  : GiveWaitBLRandAll
* Input          : Channel
* Return         : Wait time value
*******************************************************************************/
uint16_t GiveWaitBLRandAll(uint8_t channel)
{
    return ((RandomGet4() % 250)+100);
}




/*============================================================================*/
/*==================== Random - Each have it's own color =====================*/
/*============================================================================*/
/*******************************************************************************
* Function Name  : RandomEachBL
* Description    : RandomEach - random each - 6 different colors
                 : max execute time - 1.585ms
                 : mix execute time - 0.119ms
*******************************************************************************/
void RandomEachBL()
{
    for(uint8_t i=0; i<18; i++)
        SmoothProcess(RandomChannelEachBL[i], &sRandomEach);
    
    for(uint8_t i=0; i<6; i++)
    {
        outR[i] = GetSmoothCurrent(3*i+0);
        outG[i] = GetSmoothCurrent(3*i+1);
        outB[i] = GetSmoothCurrent(3*i+2);
    }   
}

/*******************************************************************************
* Function Name  : GiveTargetBLRandEach
* Input          : Channel
* Return         : Target value
*******************************************************************************/
uint8_t GiveTargetBLRandEach(uint8_t channel)
{    
    static uint8_t R[6], G[6], B[6];
    
    if(channel%3 == 0)
    {
        HSVtoRGB(&R[channel/3], &G[channel/3], &B[channel/3] , (RandomGet4() % 360));
        return R[channel/3];
    }
    if(channel%3 == 1) return G[channel/3];
    if(channel%3 == 2) return B[channel/3];
    
    return 1;
    
}

/*******************************************************************************
* Function Name  : GiveSpeedBLRandEach
* Input          : Channel
* Return         : Speed value
*******************************************************************************/
uint8_t GiveSpeedBLRandEach(uint8_t channel)
{
    return RandomGet()%5+1;
}

/*******************************************************************************
* Function Name  : GiveWaitBLRandEach
* Input          : Channel
* Return         : Wait time value
*******************************************************************************/
uint16_t GiveWaitBLRandEach(uint8_t channel)
{
    return ((RandomGet4() % 50)+20); // 700 - 20
}




/*============================================================================*/
/*============= SHIFT MODE - new color appear from left ======================*/
/*============================================================================*/
/*******************************************************************************
* Function Name  : ShiftBL
* Description    : Shift colors - from left to right - new color appear from left
*******************************************************************************/
void ShiftBL()
{
     for(uint8_t i=0; i<18; i++)
        SmoothProcess(i, &sShift);   
     
     for(uint8_t i=0; i<6; i++)    
     {
         outR[i] = GetSmoothCurrent(3*i+0);
         outG[i] = GetSmoothCurrent(3*i+1);
         outB[i] = GetSmoothCurrent(3*i+2);
     }
}

/*******************************************************************************
* Function Name  : GiveTargetBLShift
* Input          : Channel
* Return         : Target value
*******************************************************************************/
uint8_t GiveTargetBLShift(uint8_t channel)
{
    uint8_t tempR=0, tempG=0, tempB=0;
    static uint16_t cnt=0;
    
    if(channel == 0) cnt++;
        
    if(cnt >= 500)
    {
        cnt = 0;
        
        HSVtoRGB(&tempR, &tempG, &tempB, RandomGet4()%360);
        
        for(uint8_t i=17; i>2; i--) //colors shifting
            smooth[i].target = smooth[i-3].target;
                
        smooth[0].target = tempR;
        smooth[1].target = tempG;
        smooth[2].target = tempB;
    }
    
    return smooth[channel].target;
}

/*******************************************************************************
* Function Name  : GiveSpeedBLShift
* Input          : Channel
* Return         : Speed value
*******************************************************************************/
uint8_t GiveSpeedBLShift(uint8_t channel)
{
    return 3;
}

/*******************************************************************************
* Function Name  : GiveWaitBLShift
* Input          : Channel
* Return         : Wait time value 
*******************************************************************************/
uint16_t GiveWaitBLShift(uint8_t channel)
{
    return 10;
}




/*============================================================================*/
/*============= BINARY MODE - seconds->binary colors =========================*/
/*============================================================================*/
/*******************************************************************************
* Function Name  : BinarySecondsBL
* Description    : Binary backlight - seconds converts to binary backlight
*******************************************************************************/
void BinarySecondsBL()
{    
    for(uint8_t i=0; i<18; i++)
        SmoothProcess(i, &sBinary);
    
    for(uint8_t i=0; i<6; i++)
    {
        outR[i] = GetSmoothCurrent(3*i+0);
        outG[i] = GetSmoothCurrent(3*i+1);
        outB[i] = GetSmoothCurrent(3*i+2);
    }  
}

/*******************************************************************************
* Function Name  : BinaryChangeEffect
* Description    : Change color pair for binary backlight
* Input          : Direction: +1, -1
*******************************************************************************/
void BinaryChangeEffect(uint8_t direction)
{    
    if(direction == DIR_UP)
    {
        if(++binaryType >= 18)  binaryType = 0; 
    }
    else
    {
        if(binaryType == 0)     binaryType = 17;
        else                    binaryType--; 
    }
}

/*******************************************************************************
* Function Name  : GiveTargetBLBinary
* Description    : 
* Input          : Channel
* Return         : Current bright value
*******************************************************************************/
uint8_t GiveTargetBLBinary(uint8_t channel)
{
    if(BitIsSet(sec, 5-channel/3))
        return TABLE[ channel%3   ][binaryType]*255; 
    else
        return TABLE[ channel%3+3 ][binaryType]*255;
}

/*******************************************************************************
* Function Name  : GiveSpeedBLBinary
* Description    : 
* Input          : Channel
* Return         : Current bright value
*******************************************************************************/
uint8_t GiveSpeedBLBinary(uint8_t channel)
{
    return 1;
}

/*******************************************************************************
* Function Name  : GiveWaitBLBinary
* Input          : Channel
* Return         : Wait time value
*******************************************************************************/
uint16_t GiveWaitBLBinary(uint8_t channel)
{
    return 0;
}




/*******************************************************************************
* Function Name  : HSVchange
* Description    : Manual color set
* Input          : Direction: +4, -4
*******************************************************************************/
void HSVchange(uint8_t dir)           
{
    static uint16_t HSVTemp;
    
    if(dir)
    {   
        if(HSVTemp >= 356)  HSVTemp = 0;
        else                HSVTemp += 4;
    }
    else
    {
        if(HSVTemp <= 4)    HSVTemp = 360; 
        else                HSVTemp -= 4;
    } 
    
    HSVtoRGB(&ManualR, &ManualG, &ManualB, HSVTemp);
}


/*******************************************************************************
* Function Name  : Mix
* Description    : Backlight mix mode
*******************************************************************************/
void MixBL()
{
    static uint8_t mixType = 0;
    
    if(!mixTime)
    {   
        mixType = RandomGet()%5;
        
        //for Binary mode choose random binary colors
        if(mixType == 4) binaryType = RandomGet4()%18;  
        
        mixTime = eMIX_DELAY + RandomGet4()%50;
        
        /*for(uint8_t i=0; i<SMOOTH_NUM_CHANNELS; i++)
            SmoothInit(i);*/
    }
    
    switch(mixType)
    {
        case 0: FireBL();
            break;
        case 1: RandomAllBL();   
            break;
        case 2: RandomEachBL();        
            break; 
        case 3: ShiftBL();      
            break; 
        case 4: BinarySecondsBL(); 
            break; 
    }
}


/*******************************************************************************
* Function Name  : MixBLReduceTime
* Description    : Decrement seconds-counter in mix mode                     
*******************************************************************************/
void MixBLReduceTime()
{
    if(mixTime) mixTime--;   
}


/*******************************************************************************
* Function Name  : BackLightChange
* Description    : Change backlight type
*******************************************************************************/
void BackLightChange()
{
    if(++BLType >= 7) BLType = 0;   
    
    ShiftLeft(BLType);
    ShiftRight();
}


/*******************************************************************************
* Function Name  : BackLightProcess
* Description    : Backlight processing
*******************************************************************************/
void BackLightProcess()
{
    switch(BLType)
    {
        case 0: MixBL();  
            break;
        
        case 1: FireBL();               
            break;
                
        case 2: ShiftBL();
            break;
       
        case 3: BinarySecondsBL();
            break; 
        
        case 4: RandomEachBL();
            break; 
        
        case 5: RandomAllBL();
            break; 
            
        case 6: for(uint8_t k=0; k<6; k++) 
                {
                    outR[k] = ManualR; 
                    outG[k] = ManualG; 
                    outB[k] = ManualB;
                }
            break;
    }         
}


/*******************************************************************************
* Function Name  : BLGetType
* Return         : Backlight type   
*******************************************************************************/
uint8_t BLGetType()
{
    return BLType;   
}
