#include "i2c2.h"

//I2C driver
//===========================================================
//I2C setting
#define SDA_LN            PC1        //линия SDA
#define SCL_LN            PC0        //линия SCL
#define I2C_PIN           PINC       //порт входа
#define I2C_DDR           DDRC       //порт направления
#define I2C_PORT          PORTC      //порт выхода

#define I2C_T             0.00005    //период импульса
#define F_CLK             20000000
#include <intrinsics.h>
#include <ioavr.h>

unsigned char i2c_error;

// Вспомогательные функции
//============================================================================

//Устанавливает линию SDA на вход
void i2c_SdaOut(void)
{
    I2C_DDR|=(1<<SDA_LN);
    __delay_cycles(I2C_T*F_CLK);
}
//Устанавливает линию SDA на выход
void i2c_SdaIn(void)
{
    I2C_DDR&=~(1<<SDA_LN);
    __delay_cycles(I2C_T*F_CLK);
}

//Устанавливает уровень на линии SCL
void i2c_SclSetH(void)
{
    I2C_DDR&=~(1<<SCL_LN);
    I2C_PORT&=~(1<<SCL_LN);
    __delay_cycles(I2C_T*F_CLK);
}

void i2c_SclSetL(void)
{
    I2C_DDR|=(1<<SCL_LN);
    I2C_PORT&=~(1<<SCL_LN);
    __delay_cycles(I2C_T*F_CLK);
}

//Устанавливает уровень на линии SDA
void i2c_SdaSetH(void)
{
    I2C_DDR&=~(1<<SDA_LN);
    I2C_PORT&=~(1<<SDA_LN);
    __delay_cycles(I2C_T*F_CLK);
}

void i2c_SdaSetL(void)
{
    I2C_DDR|=(1<<SDA_LN);
    I2C_PORT&=~(1<<SDA_LN);
    __delay_cycles(I2C_T*F_CLK);
}

//Возвращает уровень линии SDA
unsigned char i2c_InSda(void)
{
  if(I2C_PIN&(1<<SDA_LN))
    return 1;
  else
    return 0;
}
//=======================================================================
//Основные фунции I2C

//Инициализация программной шины I2C
void i2c_Init(void)
{
  I2C_DDR&=~(1<<SDA_LN);      //изначально линии SDA
  I2C_DDR&=~(1<<SCL_LN);      //и SCL в высокоимпедансном состоянии
  I2C_PORT&=~(1<<SDA_LN);     //и на них поддерживается за счет внешних резисторов
  I2C_PORT&=~(1<<SCL_LN);     //высокий уровень
  i2c_error=0;                //изначально ошибок нет
}

//Формирует условие "СТАРТ"
void i2c_Start(void)
{
  i2c_SclSetH();
  i2c_SdaSetL();
  i2c_SclSetL();
}

//Формирует условие "СТОП"
void i2c_Stop(void)
{
  i2c_SdaSetL();
  i2c_SclSetH();
  i2c_SdaSetH();
}

//Посылка байта i2c_tx
void i2c_Send(unsigned char byte)
{
  if(i2c_error)
    return;
  for(unsigned char i=0;i<8;i++)
    {
      if(byte&0x80)
        i2c_SdaSetH();
      else
        i2c_SdaSetL();
      i2c_SclSetH();
      i2c_SclSetL();
      byte<<=1;
    }
  i2c_SdaIn();
  i2c_SclSetH();
  i2c_error=i2c_InSda();
  i2c_SclSetL();
  i2c_SdaOut();
}

//Прием байта, если last_byte=0, то принимаем последний байт и подтверждение от мастера не нужно i2c_rx
unsigned char i2c_Read(unsigned char last_byte)
{
  unsigned char data=0;
  unsigned char mask=0x80;
  if(i2c_error)
    return 0;
  i2c_SdaIn();
  for(unsigned char i=0;i<8;i++)
    {
      i2c_SclSetH();
      if(i2c_InSda())
        data=data+mask;
      mask>>=1;
      i2c_SclSetL();
    }
  i2c_SdaOut();

  if(last_byte)
  {i2c_SdaSetL();}
  else
  {i2c_SdaSetH();}

  i2c_SclSetH();
  i2c_SclSetL();
  return data;

}