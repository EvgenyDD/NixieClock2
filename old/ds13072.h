#ifndef DS13072_H
#define DS13072_H

unsigned char ds1307_dec2bcd(unsigned char num);
unsigned char ds1307_bcd2dec(unsigned char num);

unsigned char ds1307_Init(void);
void rtc_get_time(unsigned char *hour, unsigned char *min, unsigned char *sec);
void rtc_set_time(unsigned char hour,unsigned char min,unsigned char sec);
void rtc_get_date(unsigned char *date,unsigned char *month,unsigned char *year);
void rtc_set_date(unsigned char date,unsigned char month,unsigned char year);

/*
unsigned char ds1307_GetDay(void);*/

#endif