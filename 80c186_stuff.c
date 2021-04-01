#include "80c186_stuff.h"
#include "80c186eb.h"
#include "sys/80186.h"

uint8_t rd8_peri(uint16_t addr)
{
	asm("mov dx, [bp + 4]");
	asm("in al, dx");
	asm("xor ah, ah");
}

void wr8_peri(uint16_t addr, uint8_t data)
{
	asm("mov dx, [bp + 4]");
	asm("mov al, [bp + 6]");
	asm("out dx, al");
}

uint16_t rd16_peri(uint16_t addr)
{
	asm("mov dx, [bp + 4]");
	asm("in ax, dx");
}

void wr16_peri(uint16_t addr, uint16_t data)
{
	asm("mov dx, [bp + 4]");
	asm("mov ax, [bp + 6]");
	asm("out dx, ax");
}

void cout(uint8_t c)
{
	while((rd8_peri(S0STS) & 8) == 0);
	wr8_peri(T0BUF, c);
}

void dly_us(uint16_t us)
{
	us >>= 3;
	while(us--)
	{
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	}
}

void dly_ms(uint16_t ms)
{
	while(ms--)
	{
		dly_us(1000);
	}	
}
