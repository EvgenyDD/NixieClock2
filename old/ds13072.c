#include "i2c2.h"
#include "ds13072.h"
#include "all.h"
//===========================================================
//Set DS1307
#define DS1307_ADDR_W       208     //адрес микросхемы часов
#define DS1307_ADDR_R       209
#define DS1307_CONTROL      0x07    //регистр управления
#define CH                  0x07

//===========================================================
//DS1307

unsigned char ds1307_dec2bcd(unsigned char num)
{ 
   //return ((num/10 * 16) + (num % 10));
   //return (((DEC2BCD_HI(num)) << 4) + (DEC2BCD_LO(num)));
   return ((num/10) << 4)|(num%10);
}

unsigned char ds1307_bcd2dec(unsigned char num)
{ 
   //return ((num/16 * 10) + (num % 16));
   return ((((num >> 4) & 0x0F)*10) + (num & 0x0F));
}

unsigned char ds1307_Init(void)
{
  unsigned char tmp;
  i2c_Init();
  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  i2c_Send(DS1307_CONTROL);
  i2c_Send((1<<4));
  i2c_Stop();
  //---------------------------------
  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  
  i2c_Send(0);
  i2c_Stop();
  
  i2c_Start();
  i2c_Send(DS1307_ADDR_R);
  tmp=i2c_Read(0);
  i2c_Stop();
  
  if(tmp&0x80)
    return 0;
  else
    return 1;
}

void rtc_get_time(unsigned char *hour, unsigned char *min, unsigned char *sec)
{

  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  i2c_Send(0);
  i2c_Stop();
  
  i2c_Start();
  i2c_Send(DS1307_ADDR_R);
  *sec=ds1307_bcd2dec(i2c_Read(1));
  *min=ds1307_bcd2dec(i2c_Read(1));
  *hour=ds1307_bcd2dec(i2c_Read(0)); 
  i2c_Stop();
}

void rtc_set_time(unsigned char hour,unsigned char min,unsigned char sec)
{
  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  i2c_Send(0);

  i2c_Send(ds1307_dec2bcd(sec));
  i2c_Send(ds1307_dec2bcd(min));
  i2c_Send(ds1307_dec2bcd(hour));
  i2c_Stop();
}
//------------------------------------------------------------------------------
void rtc_get_date(unsigned char *date,unsigned char *month,unsigned char *year)
{

  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  i2c_Send(3);
  i2c_Stop();
  
  i2c_Start();
  i2c_Send(DS1307_ADDR_R);
  *year=ds1307_bcd2dec(i2c_Read(1)); //day of week - DUMP
  *date=ds1307_bcd2dec(i2c_Read(1)); //date
  *month=ds1307_bcd2dec(i2c_Read(1)); //number
  *year=ds1307_bcd2dec(i2c_Read(0)); //year
  
  i2c_Stop();
}
//------------------------------------------------------------------------------

/*unsigned char ds1307_GetDay(void)
{
unsigned char RetVal;
  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  
  i2c_Send(3);
  i2c_Stop();
  
  i2c_Start();
  i2c_Send(DS1307_ADDR_R);
  RetVal=i2c_Read(0);  
  i2c_Stop();
  return RetVal;
}
*/
//------------------------------------------------------------------------------
void rtc_set_date(unsigned char date,unsigned char month,unsigned char year)
{
  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  i2c_Send(3);

  i2c_Send(ds1307_dec2bcd(5)); //day of week
  i2c_Send(ds1307_dec2bcd(date));
  i2c_Send(ds1307_dec2bcd(month));
  i2c_Send(ds1307_dec2bcd(year));
  
  i2c_Stop();
}
//------------------------------------------------------------------------------