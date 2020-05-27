//#include <avr/io.h>
#include "I2c.h"

volatile unsigned char binar;

unsigned char bcd (unsigned char data)
{
	unsigned char bc;

	bc=((((data&(1<<6))|(data&(1<<5))|(data&(1<<4)))*0x0A)>>4)+((data&(1<<3))|(data&(1<<2))|(data&(1<<1))|(data&0x01));

	return bc;
}

unsigned char bin(unsigned char dec){

	char bcd;

	char n, dig, num, count;

	num = dec;
	count = 0;
	bcd = 0;
	for (n=0; n<4; n++) {
		dig = num%10;
		num = num/10;
		bcd = (dig<<count)|bcd;
		count += 4;
	}

	return bcd;
}


void rtc_init(unsigned char rs, unsigned char sqwe, unsigned char out)
{
rs&=3;
if (sqwe) rs|=0x10;
if (out) rs|=0x80;
i2c_start();
i2c_write(0xd0);
i2c_write(7);
i2c_write(rs);
i2c_stop();
}

void rtc_get_time(unsigned char *hour, unsigned char *min, unsigned char *sec)
{
i2c_start();
i2c_write(0xd0);
i2c_write(0);
i2c_stop();

i2c_start();
i2c_write(0xd1);
*sec=bcd(i2c_read(1));
i2c_stop();

i2c_start();
i2c_write(0xd1);
*min=bcd(i2c_read(1));
i2c_stop();

i2c_start();
i2c_write(0xd1);
*hour=bcd(i2c_read(1));
i2c_stop();
}

void rtc_set_time(unsigned char hour,unsigned char min,unsigned char sec)
{
i2c_start();
i2c_write(0xd0);
i2c_write(0);
i2c_write(bin(sec));
i2c_write(bin(min));
i2c_write(bin(hour));
i2c_stop();
}

void rtc_get_date(unsigned char *date,unsigned char *month,unsigned char *year)
{
i2c_start();
i2c_write(0xd0);
i2c_write(4);
i2c_stop();

i2c_start();
i2c_write(0xd1);
*date=bcd(i2c_read(1));
i2c_stop();

i2c_start();
i2c_write(0xd1);
*month=bcd(i2c_read(1));
i2c_stop();

i2c_start();
i2c_write(0xd1);
*year=bcd(i2c_read(1));
i2c_stop();
}

void rtc_set_date(unsigned char date,unsigned char month,unsigned char year)
{
i2c_start();
i2c_write(0xd0);
i2c_write(4);
i2c_write(bin(date));
i2c_write(bin(month));
i2c_write(bin(year));
i2c_stop();
}

unsigned char rtc_read(unsigned char address)
{
unsigned char data;
i2c_start();
i2c_write(0xd0);
i2c_write(address);
i2c_start();
i2c_write(0xd1);
data=i2c_read(0);
i2c_stop();
return data;
}

void rtc_write(unsigned char address,unsigned char data)
{
i2c_start();
i2c_write(0xd0);
i2c_write(address);
i2c_write(data);
i2c_stop();
}
