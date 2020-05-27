#ifndef I2C2_H
#define I2C2_H

//System function
void i2c_SdaOut(void);
void i2c_SdaIn(void);
void i2c_SclSetH(void);
void i2c_SclSetL(void);
void i2c_SdaSetH(void);
void i2c_SdaSetL(void);
unsigned char i2c_InSda(void);

void i2c_Start(void);
void i2c_Stop(void);

void i2c_Init(void);
void i2c_Send(unsigned char byte); //ѕосылка байта
unsigned char i2c_Read(unsigned char last_byte); //ѕрием байта, если last_byte=0, то принимаем последний байт и подтверждение от мастера не нужно

#endif