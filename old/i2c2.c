#include "i2c2.h"

//I2C driver
//===========================================================
//I2C setting
#define SDA_LN            PC1        //����� SDA
#define SCL_LN            PC0        //����� SCL
#define I2C_PIN           PINC       //���� �����
#define I2C_DDR           DDRC       //���� �����������
#define I2C_PORT          PORTC      //���� ������

#define I2C_T             0.00005    //������ ��������
#define F_CLK             20000000
#include <intrinsics.h>
#include <ioavr.h>

unsigned char i2c_error;

// ��������������� �������
//============================================================================

//������������� ����� SDA �� ����
void i2c_SdaOut(void)
{
    I2C_DDR|=(1<<SDA_LN);
    __delay_cycles(I2C_T*F_CLK);
}
//������������� ����� SDA �� �����
void i2c_SdaIn(void)
{
    I2C_DDR&=~(1<<SDA_LN);
    __delay_cycles(I2C_T*F_CLK);
}

//������������� ������� �� ����� SCL
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

//������������� ������� �� ����� SDA
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

//���������� ������� ����� SDA
unsigned char i2c_InSda(void)
{
  if(I2C_PIN&(1<<SDA_LN))
    return 1;
  else
    return 0;
}
//=======================================================================
//�������� ������ I2C

//������������� ����������� ���� I2C
void i2c_Init(void)
{
  I2C_DDR&=~(1<<SDA_LN);      //���������� ����� SDA
  I2C_DDR&=~(1<<SCL_LN);      //� SCL � ����������������� ���������
  I2C_PORT&=~(1<<SDA_LN);     //� �� ��� �������������� �� ���� ������� ����������
  I2C_PORT&=~(1<<SCL_LN);     //������� �������
  i2c_error=0;                //���������� ������ ���
}

//��������� ������� "�����"
void i2c_Start(void)
{
  i2c_SclSetH();
  i2c_SdaSetL();
  i2c_SclSetL();
}

//��������� ������� "����"
void i2c_Stop(void)
{
  i2c_SdaSetL();
  i2c_SclSetH();
  i2c_SdaSetH();
}

//������� ����� i2c_tx
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

//����� �����, ���� last_byte=0, �� ��������� ��������� ���� � ������������� �� ������� �� ����� i2c_rx
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