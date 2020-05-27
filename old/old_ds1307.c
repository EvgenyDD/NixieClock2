#include "i2c.h"
#include "ds1307.h"
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
 { return ((num/10 * 16) + (num % 10));}

unsigned char ds1307_bcd2dec(unsigned char num)
 { return ((num/16 * 10) + (num % 16));}

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

void ds1307_GetTime(unsigned char *MassTime)
{

  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  
  i2c_Send(0);
  i2c_Stop();
  
  i2c_Start();
  i2c_Send(DS1307_ADDR_R);

  MassTime[0]=ds1307_bcd2dec(i2c_Read(1));
  MassTime[1]=ds1307_bcd2dec(i2c_Read(1));
  MassTime[2]=ds1307_bcd2dec(i2c_Read(0));

  i2c_Stop();
}

void ds1307_SetTime(unsigned char *MassTime)
{
  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  i2c_Send(0);

  i2c_Send(ds1307_dec2bcd(MassTime[0]));
  i2c_Send(ds1307_dec2bcd(MassTime[1]));
  i2c_Send(ds1307_dec2bcd(MassTime[2]));

  i2c_Stop();
}
//------------------------------------------------------------------------------
void ds1307_GetDate(unsigned char *MassDate)
{

  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  
  i2c_Send(3);
  i2c_Stop();
  
  i2c_Start();
  i2c_Send(DS1307_ADDR_R);

  MassDate[0]=ds1307_bcd2dec(i2c_Read(1));
  MassDate[1]=ds1307_bcd2dec(i2c_Read(1));
  MassDate[2]=ds1307_bcd2dec(i2c_Read(1));
  MassDate[3]=ds1307_bcd2dec(i2c_Read(0));
  
  i2c_Stop();
}
//------------------------------------------------------------------------------
unsigned char ds1307_GetDay(void)
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
//------------------------------------------------------------------------------
void ds1307_SetDate(unsigned char *MassDate)
{
  i2c_Start();
  i2c_Send(DS1307_ADDR_W);
  i2c_Send(3);

  i2c_Send(ds1307_dec2bcd(MassDate[0]));
  i2c_Send(ds1307_dec2bcd(MassDate[1]));
  i2c_Send(ds1307_dec2bcd(MassDate[2]));
  i2c_Send(ds1307_dec2bcd(MassDate[3]));
  
  i2c_Stop();
}
//------------------------------------------------------------------------------