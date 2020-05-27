#ifndef DS1307_H
#define DS1307_H

unsigned char ds1307_dec2bcd(unsigned char num);
unsigned char ds1307_bcd2dec(unsigned char num);

unsigned char ds1307_Init(void);
void ds1307_GetTime(unsigned char *MassTime);
void ds1307_SetTime(unsigned char *MassTime);
void ds1307_GetDate(unsigned char *MassDate);
unsigned char ds1307_GetDay(void);
void ds1307_SetDate(unsigned char *MassDate);
void ds1307_GetALL(void);

#endif