/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DS1307_H
#define DS1307_H


/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define F_CPU   20000000UL
#define F_I2C   50000UL
#define TWBR_VALUE    (((F_CPU)/(F_I2C)-16)/2)

#if ((TWBR_VALUE > 255) || (TWBR_VALUE == 0))
   #error "TWBR value is not correct"
#endif

#define DS1307_ADR          0x68
#define DS1307_SEC_ADR      0x00
#define DS1307_MIN_ADR      0x01
#define DS1307_HOUR_ADR     0x02
#define DS1307_DAY_ADR      0x03
#define DS1307_DATE_ADR     0x04
#define DS1307_MONTH_ADR    0x05
#define DS1307_YEAR_ADR     0x06
#define DS1307_CTRL_ADR     0x07


/* Exported functions ------------------------------------------------------- */
unsigned char DEC2BCD(unsigned char);
unsigned char BCD2DEC(unsigned char);

void IICWrite(unsigned char, unsigned char, unsigned char);
void IICWriteInitAdr(unsigned char, unsigned char);
unsigned char IICRead(unsigned char);

void DS1307Init();
void DS1307GetTime(unsigned char*, unsigned char*, unsigned char*);
void DS1307GetDate(unsigned char*, unsigned char*, unsigned char*);
void DS1307SetTime(unsigned char, unsigned char, unsigned char);
void DS1307SetDate(unsigned char,unsigned char,unsigned char);


#endif // DS1307_H