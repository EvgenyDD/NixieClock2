/* Includes ------------------------------------------------------------------*/
#include "all.h"
#include "ds1307.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : DEC2BCD
* Input		 : Number in DEC
* Return         : Number in BCD
*******************************************************************************/
unsigned char DEC2BCD(unsigned char num)
{ 
    return ((num/10) << 4) | (num % 10);
}


/*******************************************************************************
* Function Name  : BCD2DEC
* Input		 : Number in BCD
* Return         : Number in DEC
*******************************************************************************/
unsigned char BCD2DEC(unsigned char num)
{ 
    return ((((num >> 4) & 0x0F)*10) + (num & 0x0F));
}


/*******************************************************************************
* Function Name  : IICWrite
* Description    : Write data to IIC device
* Input		 : Device address, Register address ,data
*******************************************************************************/
void IICWrite(unsigned char deviceAdr, unsigned char registerAdr, unsigned char data)
{
//start
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
//ADR_Wr
    TWDR = (deviceAdr<<1)|0;
    TWCR = (1<<TWINT)|(1<<TWEN); 
    while(!(TWCR & (1<<TWINT)));
//register adress
    TWDR = registerAdr;
    TWCR = (1<<TWINT)|(1<<TWEN); 
    while(!(TWCR & (1<<TWINT)));
//data
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN); 
    while(!(TWCR & (1<<TWINT)));
//stop
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);  
}


/*******************************************************************************
* Function Name  : IICWriteInitAdr
* Description    : Write init address to IIC device
* Input		 : Device address, Register address
*******************************************************************************/
void IICWriteInitAdr(unsigned char deviceAdr, unsigned char adr)
{
//start
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
//ADR_Wr
    TWDR = (deviceAdr<<1)|0;
    TWCR = (1<<TWINT)|(1<<TWEN); 
    while(!(TWCR & (1<<TWINT)));
//register adress
    TWDR = adr;
    TWCR = (1<<TWINT)|(1<<TWEN); 
    while(!(TWCR & (1<<TWINT)));
//stop
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);  
}


/*******************************************************************************
* Function Name  : IICRead
* Description    : Read data from IIC device - after init address was written
* Input		 : Device address
*******************************************************************************/
unsigned char IICRead(unsigned char deviceAdr)
{
    unsigned char data;
    
//start
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));  
//ADR_Rd
    TWDR = (deviceAdr<<1)|1;
    TWCR = (1<<TWINT)|(1<<TWEN); 
    while(!(TWCR & (1<<TWINT)));
//get data
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
    data = TWDR;
//stop
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN); 
    
    return data;
}


/*******************************************************************************
* Function Name  : DS1307Init
* Description    : Initialize IIC bus and enable DS1307 1Hz SquareWaveOutput
*******************************************************************************/
void DS1307Init()
{  
//IIC Init
    TWBR = TWBR_VALUE;
    TWSR = 0;
    
//Enable 1Hz SquareWaveOutput
    IICWrite(DS1307_ADR, DS1307_CTRL_ADR, 1<<4);    
}


/*******************************************************************************
* Function Name  : DS1307GetTime
* Description    : Get time from DS1307
* Input		 : pointers to hour, minute, second
*******************************************************************************/
void DS1307GetTime(unsigned char *hour, unsigned char *min, unsigned char *sec)
{    
    IICWriteInitAdr(DS1307_ADR, DS1307_SEC_ADR);
    *sec =  BCD2DEC( IICRead(DS1307_ADR) );
    *min =  BCD2DEC( IICRead(DS1307_ADR) );
    *hour = BCD2DEC( IICRead(DS1307_ADR) );
}


/*******************************************************************************
* Function Name  : DS1307GetDate
* Description    : Get date from DS1307
* Input		 : pointers to date, month, year
*******************************************************************************/
void DS1307GetDate(unsigned char *date, unsigned char *month, unsigned char *year)
{    
    IICWriteInitAdr(DS1307_ADR, DS1307_DATE_ADR);
    *date = BCD2DEC( IICRead(DS1307_ADR) );
    *month= BCD2DEC( IICRead(DS1307_ADR) );
    *year = BCD2DEC( IICRead(DS1307_ADR) );       
}


/*******************************************************************************
* Function Name  : Ds1307SetTime
* Description    : Write time to DS1307
* Input		 : Hour, minute, second
*******************************************************************************/
void DS1307SetTime(unsigned char hour, unsigned char min, unsigned char sec)
{
    IICWrite(DS1307_ADR, DS1307_SEC_ADR, DEC2BCD(sec));
    IICWrite(DS1307_ADR, DS1307_MIN_ADR, DEC2BCD(min));
    IICWrite(DS1307_ADR, DS1307_HOUR_ADR, DEC2BCD(hour));    
}


/*******************************************************************************
* Function Name  : DS1307SetDate
* Description    : Write time to DS1307
* Input		 : Date, month, year
*******************************************************************************/
void DS1307SetDate(unsigned char date,unsigned char month,unsigned char year)
{    
    IICWrite(DS1307_ADR, DS1307_DATE_ADR, DEC2BCD(date));
    IICWrite(DS1307_ADR, DS1307_MONTH_ADR, DEC2BCD(month));
    IICWrite(DS1307_ADR, DS1307_YEAR_ADR, DEC2BCD(year));  
}