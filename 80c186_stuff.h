#ifndef __80C186_STUFF_H
#define __80C186_STUFF_H
#include <stdint.h>
#include "80c186eb.h"

uint8_t rd8_peri(uint16_t addr);
void wr8_peri(uint16_t addr, uint8_t data);
void wr16_peri(uint16_t addr, uint16_t data);
uint16_t rd16_peri(uint16_t addr);

void dly_us(uint16_t us);
void dly_ms(uint16_t ms);

#define cin_stat() rd8_peri(S0STS)
#define cin_kbhit() (cin_stat() & 0x40)
#define cin_buf() rd8_peri(R0BUF)

#endif // __80C186_STUFF_H
