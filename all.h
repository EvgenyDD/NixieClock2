/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ALL_H
#define ALL_H


/* Includes ------------------------------------------------------------------*/
#include <ioavr.h>
#include <intrinsics.h>
#include <stdlib.h>

/* Exported types ------------------------------------------------------------*/
typedef unsigned char uint8_t; 
typedef unsigned int uint16_t;


/* Exported constants --------------------------------------------------------*/
#define FREQ 20000000UL
#define _delay_us(us)          __delay_cycles((FREQ/1000000)*(us))
#define _delay_ms(ms)          __delay_cycles((FREQ/1000)*(ms))
#define _delay_s(s)            __delay_cycles((FREQ)*(s))


/* Exported macro ------------------------------------------------------------*/
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (m))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)

#define SQW_IN                 PIND & (1 << PD2)


/* Exported define -----------------------------------------------------------*/
#define BLANK 10
enum {DIR_DOWN, DIR_UP};


/* Exported functions ------------------------------------------------------- */
uint16_t RandomGet();
uint16_t RandomGet4();
  

#endif //ALL_H