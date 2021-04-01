#include "or566_base.h"
#include "80c186_stuff.h"
#include "80c186eb.h"
#include "sys/80186.h"


void set_leds(uint8_t leds)
{
	wr16_peri(0x2002, leds);
}

uint8_t get_in_u8(void)
{
	return rd8_peri(0x2003);
}

uint8_t get_in_u4(void)
{
	return rd8_peri(0x2001);
}