#ifndef ALL_H
#define ALL_H

#include <ioavr.h>
#include <intrinsics.h>

typedef unsigned char u8; 
typedef unsigned int u16;

#define FREQ 20000000UL
#define _delay_uus(us)          __delay_cycles((FREQ/1000000)*(us))
#define _delay_ms(ms)          __delay_cycles((FREQ/1000)*(ms))
#define _delay_s(s)            __delay_cycles((FREQ)*(s))

#define SQW_IN PIND & (1 << PD2)

#define BitIsSet(reg, bit)        ((reg & (1<<bit)) != 0)
#define BitIsClear(reg, bit)      ((reg & (1<<bit)) == 0)
#define ClearBit(reg, bit)        reg &= (~(1<<(bit))) 
#define SetBit(reg, bit)          reg |= (1<<(bit))  

#endif