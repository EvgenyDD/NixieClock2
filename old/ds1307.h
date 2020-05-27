#ifndef DS1307_H_
#define DS1307_H_

void rtc_init(unsigned char rs, unsigned char sqwe, unsigned char out);
void rtc_get_time(unsigned char *hour, unsigned char *min, unsigned char *sec);
void rtc_get_date(unsigned char *date, unsigned char *month, unsigned char *year);
void rtc_set_time(unsigned char hour, unsigned char min, unsigned char sec);
void rtc_set_date(unsigned char date, unsigned char month, unsigned char year);
unsigned char rtc_read(unsigned char address);
void rtc_write(unsigned char address,unsigned char data);

#endif /* DS1307_H_ */