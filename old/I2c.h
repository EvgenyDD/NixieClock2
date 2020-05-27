#include "all.h"
#define I2C_DELAY 1

#define I2C_READ 1
#define I2C_WRITE 0

#define I2C_ACK 0
#define I2C_NACK 1

#define I2C_WAIT 1

void i2c_init();
void i2c_start();
void i2c_start_rep();
void i2c_stop();
signed char  i2c_write(unsigned char  byte);
unsigned char  i2c_read(unsigned char  ack);
void i2c_address(unsigned char address, unsigned char  rw, unsigned char  wait);
