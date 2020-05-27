//#include  <avr/io.h>
//#include  <util/delay.h>
//#include "stdio.h"
//#include "stdlib.h"
#include "I2c.h"

#define I2C_SDA_PIN            PC1                         //линия SDA
#define I2C_SCL_PIN            PC0                         //линия SCL

#define I2C_SDA_PORT_READ           PINC                        //порт входа

#define I2C_SDA_PORT_DIR           DDRC                        //порт направления
#define I2C_SCL_PORT_DIR           DDRC                        //порт направления
#define I2C_SDA_PORT          PORTC                       //порт выхода
#define I2C_SCL_PORT          PORTC                       //порт выхода

#define SET(reg, bit) (reg |= (1 << bit))
#define CLR(reg, bit) (reg &= ~(1 << bit))
#define GETBIT(byte, bit) ((byte >> bit) & 1)
#define SETBIT(byte, bit) (byte | (1 << bit))

#define I2C_SDA_LOW (SET(I2C_SDA_PORT_DIR, I2C_SDA_PIN))
#define I2C_SDA_HIGH (CLR(I2C_SDA_PORT_DIR, I2C_SDA_PIN))
#define I2C_SCL_LOW (SET(I2C_SCL_PORT_DIR, I2C_SCL_PIN))
#define I2C_SCL_HIGH (CLR(I2C_SCL_PORT_DIR, I2C_SCL_PIN))

#define I2C_SDA_VALUE (GETBIT(I2C_SDA_PORT_READ, I2C_SDA_PIN))

#define _delay_us(n)  __delay_cycles(8*(unsigned short)n)


void i2c_init() {
  CLR(I2C_SDA_PORT, I2C_SDA_PIN);
  CLR(I2C_SCL_PORT, I2C_SCL_PIN);
  I2C_SDA_HIGH;
  I2C_SCL_HIGH;
}

void i2c_start() {
  I2C_SDA_LOW;
  _delay_us(I2C_DELAY);
}

void i2c_start_rep() {
  I2C_SCL_LOW;
  _delay_us(I2C_DELAY);
  I2C_SDA_HIGH;
  _delay_us(I2C_DELAY);
  I2C_SCL_HIGH;
  _delay_us(I2C_DELAY);
  I2C_SDA_LOW;
  _delay_us(I2C_DELAY);
}

void i2c_stop() {
  I2C_SCL_LOW;
  I2C_SDA_LOW;
  _delay_us(I2C_DELAY);
  I2C_SCL_HIGH;
  _delay_us(I2C_DELAY); 
  I2C_SDA_HIGH;
  _delay_us(I2C_DELAY);
}

signed char  i2c_write(unsigned char  byte) {
  unsigned char  i;
  for(i = 0; i < 8; i ++) {
    I2C_SCL_LOW;
    if(GETBIT(byte, 7)) {
      I2C_SDA_HIGH;
    } else {
      I2C_SDA_LOW;
    }
    _delay_us(I2C_DELAY);
    I2C_SCL_HIGH;
    _delay_us(I2C_DELAY);
    byte <<= 1;
  }
  I2C_SCL_LOW;
  I2C_SDA_HIGH;
  _delay_us(I2C_DELAY);
  I2C_SCL_HIGH;
  _delay_us(I2C_DELAY);
  if(I2C_SDA_VALUE == 1) {
    return -1;
  }
  return 0;
}

unsigned char  i2c_read(unsigned char  ack) {
  I2C_SCL_LOW;
  I2C_SDA_HIGH;
  _delay_us(I2C_DELAY);
  I2C_SCL_HIGH;
  _delay_us(I2C_DELAY);
  unsigned char  i, result = I2C_SDA_VALUE; 
  for(i = 0; i < 7; i ++) {
    I2C_SCL_LOW;
    _delay_us(I2C_DELAY);
    I2C_SCL_HIGH;
    _delay_us(I2C_DELAY);
    result <<= 1;
    if(I2C_SDA_VALUE == 1) {
      result |= 1;
    }
  }
  I2C_SCL_LOW;
  if(ack == I2C_ACK) {
    I2C_SDA_LOW;
  } else {
    I2C_SDA_HIGH;
  }
  _delay_us(I2C_DELAY);
  I2C_SCL_HIGH;
  _delay_us(I2C_DELAY);
  return result; 
}

void i2c_address(unsigned char address, unsigned char  rw, unsigned char  wait) {
  if(wait == I2C_WAIT) {
    while(i2c_write((address << 1) + rw) == -1) {
      i2c_stop();
      i2c_start();
    }
  } else {
    i2c_write((address << 1) + rw);
  }
}
