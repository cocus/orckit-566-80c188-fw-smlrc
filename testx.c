#include <stdint.h>
#include "80c186eb.h"
#include "sys/80186.h"
#include "or566_base.h"
#include "80c186_stuff.h"


/*#define S0STS 0xFF66 //0FF66H
#define R0BUF 0xFF68 //0FF68H
#define T0BUF 0xFF6A //0FF6AH*/
/*uint8_t cin_stat(void)
{
	return rd8_peri(S0STS);
}*/



/*uint8_t cin_buf(void)
{
	return rd8_peri(R0BUF);
}*/


void crlf(void)
{
	cout('\r'); cout('\n');
}

#if TEST_RODATA
void puts(char * str)
{
	while(*str)
	{
		cout(*str);
		str++;
	}
}
char dstr[] = ".data";
int baint[5];

signed char sc = -1;
unsigned char uc = 255;

void serial_itoa(int n)
{
  char i,j;                    //decade counter
  //char idx=0;                  //string index     
  //int const Div[4] = {      //decades table
  //  1000000000L,100000000L,10000000L,1000000L
  //  100000L,10000L,1000L,100L,10L,1};
  int b;                     //i32 to hold table read                           
  char fdd;                     //first digit detect (suppress leading zero function)
 
  fdd=0;                       //clear detection
  if (n & 0x8000) {        //T: n=negative
    n=(~n)+1;                  //convert to positive
    //s[0]='-';                  //mark the negative number
	cout('-');
    //idx++;
  } 
 
  for (i=0; i<5;i++) {        //do all the decades, start with biggest
    j=0;                       //clear the decimal digit counter
	switch (i)
	{
		case 0: b = 10000; break;
		case 1: b = 1000; break;
		case 2: b = 100; break;
		case 3: b = 10; break;
		case 4: b = 1; break;
	}
    while (n>=b) {             //T: "left-over" still bigger then decade; substr. and count
      j++;
      n-=b;
    } 
    if ((j)||(fdd)) {          //T: decade count!=0 or first digit has been detected
      fdd=1;
	  cout('0' + j);
    } 
  }

  if (!fdd) cout('0');
}

void serial_hexnum4(uint8_t n)
{
	if (n > 9)
	{
		n = 'A' + (n - 10);
	}
	else
	{
		n = '0' + n;
	}
	cout(n);
}

void serial_hexnum8(uint8_t n)
{
	serial_hexnum4(n >> 4);
	serial_hexnum4(n & 0xf);
}

void serial_hexnum16(uint16_t n)
{
	serial_hexnum8(n >> 8);
	serial_hexnum8(n & 0xff);
}

void serial_hexdump(uint8_t *addr, uint16_t cnt)
{
	uint16_t i = 0;
	uint8_t chr_idx = 0;
	uint8_t dsp[17] = { 0 };

	while (cnt--)
	{
		if (i % 16 == 0)
		{
			if (i != 0)
			{
				puts(" |"); puts(dsp); puts("|");
				for (chr_idx = 0; chr_idx < 16; chr_idx++)
				{
					dsp[chr_idx] = '\0';
				}
			}
			crlf();
			serial_hexnum16(addr); cout(':');
		}
		serial_hexnum8(*addr); cout(' ');
		if ((*addr >= 0x20) &&
			(*addr <= 0x70))
		{
			dsp[i%16] = *addr;
		}
		else
		{
			dsp[i%16] = '.';
		}
		addr++;
		i++;
	}
	puts(" |"); puts(dsp); puts("|");


	crlf();
}

#endif


void init_gcs(uint8_t nbr, uint16_t start, uint16_t end)
{

	asm("mov dl, [bp + 4]");		/* DL = nbr */
	asm("sal dl, 02h");				/* DL = DL * 4 */
	asm("add dl, 080h");			/* DL = DL + 0x80 (GCS0ST base) */
	asm("mov dh, 0FFh");			/* DH = 0xFF */
	asm("mov ax, [bp + 6]");		/* AX = start */
	asm("or al, 0fh");				/* AL = AL | 0xF (16 wait states) */
	asm("out dx, ax");
	asm("mov ax, [bp + 8]");		/* AX = end */
	asm("or al, 08h");				/* AL = AL | 0x8 (IO, enabled, no RDY) */
	//asm("mov ax, 02048H");
	asm("add dl, 02h");				/* DL = DL + 2 (now points to GCSxSP) */
	asm("out dx, ax");
}


void print_bin(uint8_t bin)
{
    uint8_t i;

    for(i = 0x80; i != 0; i >>= 1)
	{
        cout((bin & i)?'1':'0');
	}
	crlf();
}

#ifdef SUPPORT_FOR_I2C
#define i2c_delay() //asm("nop");asm("nop");asm("nop");asm("nop");

void i2c_sda_0(void)
{
	/* Set I2C's SDA line OFF (i.e. tied to ground) */
	wr16_peri(P2LTCH, rd16_peri(P2LTCH) & 0x00BF);
}

void i2c_sda_1(void)
{
	/* Set I2C's SDA line ON (i.e. not tied to ground) */
	wr16_peri(P2LTCH, rd16_peri(P2LTCH) | 0x0040);
}

void i2c_scl_0(void)
{
	/* Set I2C's SCL line OFF */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) & 0x00FD);
}

void i2c_scl_1(void)
{
	/* Set I2C's SCL line ON */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) | 0x0002);
}

uint8_t i2c_sample_sda(void)
{
	return (rd16_peri(P2PIN) & 0x40) ? 1 : 0;
}

void init_i2c(void)
{
	/* Turn PC1CON's PC1 bit off, making P1.1 IO */
	wr16_peri(P1CON, rd16_peri(P1CON) & 0x00FD);
	/* Turn PC2CON's PC6 bit off, making P2.6 IO */
	wr16_peri(P2CON, rd16_peri(P2CON) & 0x00BF);


	wr16_peri(P1DIR, rd16_peri(P1DIR) & 0x00FD);
	wr16_peri(P2DIR, rd16_peri(P2DIR) & 0x00BF);


	i2c_scl_1();
	i2c_sda_1();
}

uint8_t i2c_uint8_t_out(uint8_t data)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
		{
			i2c_sda_1(); // set data line to 1
		}
		else
		{
			i2c_sda_0(); // set data line to 0
		}
		i2c_scl_1(); // clock high (slave latches data)
		i2c_delay();
		i2c_scl_0(); // clock low
		data <<= 1;
	}

	i2c_sda_1(); // set data line for reading ack bit
	i2c_scl_1(); // clock line high
	i2c_delay();
	i = i2c_sample_sda(); // read ACK bit
	i2c_scl_0(); // clock low
	i2c_delay();
	i2c_sda_0(); // data low

	return (i == 0) ? 1 : 0;
}

uint8_t i2c_uint8_t_in(uint8_t is_last)
{
	uint8_t i;
	uint8_t b = 0;

	i2c_sda_1(); // set data line as input

	for(i = 0; i < 8; i++)
	{
		i2c_delay(); // wait for data to settle
		i2c_scl_1(); // clock high (slave latches data)
		i2c_delay();
		b <<= 1;
		if (i2c_sample_sda()) // read the data bit
		{
			b |= 1; // set data bit
		}
		i2c_scl_0(); // clock low
	}
	if (is_last)
	{
		i2c_sda_1(); // last uint8_t sends a NACK
	}
	else
	{
		i2c_sda_0(); // otherwise, ACK
	}

	i2c_scl_1(); // clock high
	i2c_delay();
	i2c_scl_0(); // clock low to send ack/nack
	i2c_delay();
	i2c_sda_0(); // // data low
	
	return b;
}


uint8_t i2c_send_start(uint8_t addr, uint8_t read)
{
	i2c_sda_0(); // data line low first
	i2c_delay();
	i2c_scl_0(); // then clock line low is a START signal

	addr <<= 1;
	if (read)
	{
		addr |= 1;
	}

	return i2c_uint8_t_out(addr);
}

void i2c_send_stop(void)
{
	i2c_sda_0(); // data line low
	i2c_delay();
	i2c_scl_1(); // clock high
	i2c_delay();
	i2c_sda_1(); // data high
	i2c_delay();
}

uint8_t eeprom_24c16_read(uint8_t dev_addr, uint8_t word_addr, uint8_t *buffer, uint16_t count)
{
	uint8_t status;
	if (i2c_send_start(dev_addr, 0))
	{
		if (i2c_uint8_t_out(word_addr))
		{
			i2c_send_stop();
			if (i2c_send_start(dev_addr, 1))
			{
				while(count--)
				{
					*buffer++ = i2c_uint8_t_in(count == 0);
				}
				i2c_send_stop();
				return 1;
			}
		}
	}
	return 0;
}

#endif // #ifdef SUPPORT_FOR_I2C


#if 1

#define spi_delay() 

void spi_mosi_0(void)
{
	/* MOSI: P1.0 OFF */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) & 0x00FE);
}

void spi_mosi_1(void)
{
	/* MOSI: P1.0 ON */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) | 0x0001);
}

void spi_sck_0(void)
{
	/* SCK: P1.1 OFF */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) & 0x00FD);
}

void spi_sck_1(void)
{
	/* SCK: P1.1 ON */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) | 0x0002);
}

void spi_cs_0(void)
{
	/* CS: P1.5 OFF */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) & 0x00DF);
}

void spi_cs_1(void)
{
	/* CS: P1.5 ON */
	wr16_peri(P1LTCH, rd16_peri(P1LTCH) | 0x0020);
}

uint8_t spi_sample_miso(void)
{
	return (rd16_peri(P2PIN) & 0x40) ? 1 : 0;
}

void init_spi(void)
{
	/**
	 * mod SD card:
	 * MISO: P2.6
	 * MOSI: P1.0
	 * SCK: P1.1
	 * CS: P1.5
	 */

	/* PC0, PC1 and PC5 bits off, P1.0, P1.1 and P1.5 IO */
	wr16_peri(P1CON, rd16_peri(P1CON) & 0x00DC);
	/* Turn PC2CON's PC6 bit off, making P2.6 IO */
	wr16_peri(P2CON, rd16_peri(P2CON) & 0x00BF);

	/* P1.0, P1.1, P1.5 => Output */
	wr16_peri(P1DIR, rd16_peri(P1DIR) & 0x00DC);
	/* P2.6 => Input */
	wr16_peri(P2DIR, rd16_peri(P2DIR) | 0x0040);

	spi_cs_1();
	spi_sck_0();
}

uint8_t spi_txrx_byte(uint8_t data)
{
	uint8_t i;
	uint8_t b = 0;

	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
		{
			spi_mosi_1(); // set data line to 1
		}
		else
		{
			spi_mosi_0(); // set data line to 0
		}
		spi_sck_1(); // clock high
		spi_delay();
		b <<= 1;
		if (spi_sample_miso()) // read the data bit
		{
			b |= 1; // set data bit
		}
		spi_sck_0(); // clock low
		data <<= 1;
	}

	return b;
}

/* Definitions of SD commands */
#define CMD0    (0x40+0)        /* GO_IDLE_STATE            */
#define CMD1    (0x40+1)        /* SEND_OP_COND (MMC)       */
#define ACMD41  (0xC0+41)       /* SEND_OP_COND (SDC)       */
#define CMD8    (0x40+8)        /* SEND_IF_COND             */
#define CMD9    (0x40+9)        /* SEND_CSD                 */
#define CMD16   (0x40+16)       /* SET_BLOCKLEN             */
#define CMD17   (0x40+17)       /* READ_SINGLE_BLOCK        */
#define CMD24   (0x40+24)       /* WRITE_SINGLE_BLOCK       */
#define CMD42   (0x40+42)       /* LOCK_UNLOCK              */
#define CMD55   (0x40+55)       /* APP_CMD                  */
#define CMD58   (0x40+58)       /* READ_OCR                 */
#define CMD59   (0x40+59)       /* CRC_ON_OFF               */

#define SD_INIT_TRYS    0x03

/* CardType) */
#define SDCT_MMC        0x01                    /* MMC version 3    */
#define SDCT_SD1        0x02                    /* SD version 1     */
#define SDCT_SD2        0x04                    /* SD version 2     */
#define SDCT_SDC        (SDCT_SD1|SDCT_SD2)     /* SD               */
#define SDCT_BLOCK      0x08                    /* Block addressing */

#define SD_BLK_SIZE     512

/* Results of SD functions */
typedef enum {
    SD_OK = 0,      /* 0: Function succeeded    */
    SD_NOINIT,      /* 1: SD not initialized    */
    SD_ERROR,       /* 2: Disk error            */
    SD_PARERR,      /* 3: Invalid parameter     */
    SD_BUSY,        /* 4: Programming busy      */
    SD_REJECT,      /* 5: Reject data           */
    SD_NORESPONSE   /* 6: No response           */
} SDRESULTS;

/* SD device object */
typedef struct _SD_DEV {
    uint8_t mount;
    uint8_t cardtype;
    uint16_t last_sector_lo;
    uint16_t last_sector_hi;
#ifdef SD_IO_DBG_COUNT
    DBG_COUNT debug;
#endif
} SD_DEV;

uint8_t __SD_Send_Cmd(uint8_t cmd, uint16_t arg_lo, uint16_t arg_hi)
{
    uint8_t crc, res;
    // ACMD«n» is the command sequense of CMD55-CMD«n»
    if(cmd & 0x80) {
        cmd &= 0x7F;
        res = __SD_Send_Cmd(CMD55, 0, 0);
        if (res > 1) return (res);
    }

    // Select the card
    spi_cs_1();
    spi_txrx_byte(0xFF);
    spi_cs_0();
    spi_txrx_byte(0xFF);

    // Send complete command set
    spi_txrx_byte(cmd);                        // Start and command index
    spi_txrx_byte((uint8_t)(arg_hi >> 8));          // Arg[31-24]
    spi_txrx_byte((uint8_t)(arg_hi >> 0));          // Arg[23-16]
    spi_txrx_byte((uint8_t)(arg_lo >> 8 ));          // Arg[15-08]
    spi_txrx_byte((uint8_t)(arg_lo >> 0 ));          // Arg[07-00]

    // CRC?
    crc = 0x01;                         // Dummy CRC and stop
    if(cmd == CMD0) crc = 0x95;         // Valid CRC for CMD0(0)
    if(cmd == CMD8) crc = 0x87;         // Valid CRC for CMD8(0x1AA)
    spi_txrx_byte(crc);

    // Receive command response
    // Wait for a valid response in timeout of 5 milliseconds
    //SPI_Timer_On(5);
    do {
        res = spi_txrx_byte(0xFF);
    } while((res & 0x80));
    //SPI_Timer_Off();
    // Return with the response value
    return(res);
}


void longxlong(uint16_t a1, uint16_t a2, uint16_t b1, uint16_t b2, uint16_t *ret1, uint16_t *ret2)
{
 	/* from https://stackoverflow.com/posts/60015995 */
    asm("mov     ax, [bp + 8]"); // b1
    asm("mov     cx, ax");
    asm("mul     word [bp + 6]"); // a2
    asm("mov     bx, ax            ; save  y_lo * x_hi");
    asm("mov     ax, [bp + 4]"); // a1
    asm("mov     di, ax");
    asm("mul     word [bp + 10]"); // b2
    asm("add     bx, ax            ; sum of the cross products");
    asm("mov     ax, di");
    asm("mul     cx                ; DX:AX = y_lo * x_lo");
    asm("add     dx, bx            ; add the cross products into the high half");

	asm("mov bx, word [bp + 12]"); // ret1
	asm("mov word [bx], ax"); // *ret1=ax

	asm("mov bx, word [bp + 14]"); // ret2
	asm("mov word [bx], dx"); // *ret2=dx
}

void long_d_int(uint16_t a1, uint16_t a2, uint16_t divisor, uint16_t *ret1, uint16_t *ret2, uint16_t *remainder)
{
	asm("mov     cx,[bp + 8]            ;cx = dvsr");
	asm("xor     dx,dx                  ;dx = 0");
	asm("mov     ax,word [bp + 6]    ;ax = high order numerator");
	asm("div     cx                     ;dx = rem, ax = high order quotient");
	asm("mov bx, word [bp + 12]");
	asm("mov     word [bx],ax   ;store high order quotient");
	asm("mov     ax,word [bp + 4]      ;ax = low  order numerator");
	asm("div     cx                     ;dx = rem, ax = low  order quotient");
	asm("mov bx, word [bp + 10]");
	asm("mov     word [bx],ax     ;store low  order quotient");
	if (remainder)
	{
		asm("mov bx, word [bp + 14]");
		asm("mov     word [bx],dx     ;store remainder");
	}
}

void __SD_Power_Of_Two(uint8_t e, uint16_t *ret_lo, uint16_t *ret_hi)
{
    uint16_t partial_lo = 1;
    uint16_t partial_hi = 0;
    uint8_t idx;
    for(idx=0; idx!=e; idx++)
	{
		longxlong(partial_lo, partial_hi, 2, 0, &partial_lo, &partial_hi);
	}

	*ret_lo = partial_lo;
	*ret_hi = partial_hi;
}

SDRESULTS __SD_Sectors (SD_DEV *dev, uint16_t *sectors_lo, uint16_t *sectors_hi)
{
    uint8_t csd[16];
    uint8_t idx;
    uint16_t ss_lo = 0;
    uint16_t ss_hi = 0;
	uint16_t p_two_lo, p_two_hi;
    uint16_t C_SIZE = 0;
    uint8_t C_SIZE_MULT = 0;
    uint8_t READ_BL_LEN = 0;
    if(__SD_Send_Cmd(CMD9, 0, 0)==0) 
    {
        // Wait for response
        while (spi_txrx_byte(0xFF) == 0xFF);
        for (idx=0; idx!=16; idx++) csd[idx] = spi_txrx_byte(0xFF);
        // Dummy CRC
        spi_txrx_byte(0xFF);
        spi_txrx_byte(0xFF);
        //SPI_Release();
        if(dev->cardtype & SDCT_SD1)
        {
            ss_lo = csd[0];
			ss_hi = 0;
            // READ_BL_LEN[83:80]: max. read data block length
            READ_BL_LEN = (csd[5] & 0x0F);
            // C_SIZE [73:62]
            C_SIZE = (csd[6] & 0x03);
            C_SIZE <<= 8;
            C_SIZE |= (csd[7]);
            C_SIZE <<= 2;
            C_SIZE |= ((csd[8] >> 6) & 0x03);
            // C_SIZE_MULT [49:47]
            C_SIZE_MULT = (csd[9] & 0x03);
            C_SIZE_MULT <<= 1;
            C_SIZE_MULT |= ((csd[10] >> 7) & 0x01);
        }
        else if(dev->cardtype & SDCT_SD2)
        {
            // C_SIZE [69:48]
            C_SIZE = (csd[7] & 0x3F);
            C_SIZE <<= 8;
            C_SIZE |= (csd[8] & 0xFF);
            C_SIZE <<= 8;
            C_SIZE |= (csd[9] & 0xFF);
            // C_SIZE_MULT [--]. don't exits
            C_SIZE_MULT = 0;
        }
        ss_lo = (C_SIZE + 1);
		//ss *= __SD_Power_Of_Two(C_SIZE_MULT + 2);
		__SD_Power_Of_Two(C_SIZE_MULT + 2, &p_two_lo, &p_two_hi);
		longxlong(ss_lo, ss_hi, p_two_lo, p_two_hi, &ss_lo, &ss_hi);
        
        //ss *= __SD_Power_Of_Two(READ_BL_LEN);
		__SD_Power_Of_Two(READ_BL_LEN, &p_two_lo, &p_two_hi);
		longxlong(ss_lo, ss_hi, p_two_lo, p_two_hi, &ss_lo, &ss_hi);

        //ss /= SD_BLK_SIZE;
		long_d_int(ss_lo, ss_hi, SD_BLK_SIZE, &ss_lo, &ss_hi, 0);
        return (SD_OK);
    } else return (SD_ERROR); // Error
}

#define delay_ms(x) dly_ms(x)
#define MMCSD_SPI_XFER(x) spi_txrx_byte(x)
#define mmcsd_deselect() spi_cs_1()
#define mmcsd_select() spi_cs_0()

typedef enum
{
	MMCSD_GOODEC = 0,
	MMCSD_IDLE = 0x01,
	MMCSD_ERASE_RESET = 0x02,
	MMCSD_ILLEGAL_CMD = 0x04,
	MMCSD_CRC_ERR = 0x08,
	MMCSD_ERASE_SEQ_ERR = 0x10,
	MMCSD_ADDR_ERR = 0x20,
	MMCSD_PARAM_ERR = 0x40,
	RESP_TIMEOUT = 0x80
} MMCSD_err;

#define GO_IDLE_STATE 0
#define SEND_OP_COND 1
#define SEND_IF_COND 8
#define SEND_CSD 9
#define SEND_CID 10
#define SD_STATUS 13
#define SEND_STATUS 13
#define SET_BLOCKLEN 16
#define READ_SINGLE_BLOCK 17
#define WRITE_BLOCK 24
#define SD_SEND_OP_COND 41
#define APP_CMD 55
#define READ_OCR 58
#define CRC_ON_OFF 59

#define IDLE_TOKEN 0x01
#define DATA_START_TOKEN 0xFE

#define MMCSD_MAX_BLOCK_SIZE 512

uint8_t g_CRC_enabled;


uint8_t mmcsd_crc7(char *data,uint8_t length)
{
   uint8_t i, ibit, c, crc;
    
   crc = 0x00;                                                                // Set initial value

   for (i = 0; i < length; i++, data++)
   {
      c = *data;

      for (ibit = 0; ibit < 8; ibit++)
      {
         crc = crc << 1;
         if ((c ^ crc) & 0x80) crc = crc ^ 0x09;                              // ^ is XOR
         c = c << 1;
      }

       crc = crc & 0x7F;
   }

   //shift_left(&crc, 1, 1);                                                    // MMC card stores the result in the top 7 bits so shift them left 1
                                                                              // Should shift in a 1 not a 0 as one of the cards I have won't work otherwise
	crc >>= 1;																			  
   return crc;
}

MMCSD_err mmcsd_send_cmd(uint8_t cmd, uint16_t arg_lo, uint16_t arg_hi)
{
   uint8_t packet[6]; // the entire command, argument, and crc in one variable

   // construct the packet
   // every command on an SD card is or'ed with 0x40
   packet[0] = cmd | 0x40;
   packet[1] = (uint8_t)(arg_hi >> 8);
   packet[2] = (uint8_t)(arg_hi >> 0);
   packet[3] = (uint8_t)(arg_lo >> 8);
   packet[4] = (uint8_t)(arg_lo >> 0);

   // calculate the crc if needed
   if(g_CRC_enabled)
      packet[5] = mmcsd_crc7(packet, 5);
   else
      packet[5] = 0xFF;

   // transfer the command and argument, with an extra 0xFF hacked in there
   MMCSD_SPI_XFER(packet[0]);
   MMCSD_SPI_XFER(packet[1]);
   MMCSD_SPI_XFER(packet[2]);
   MMCSD_SPI_XFER(packet[3]);
   MMCSD_SPI_XFER(packet[4]);
   MMCSD_SPI_XFER(packet[5]);
//!   spi_write2(packet[0]);
//!   spi_write2(packet[1]);
//!   spi_write2(packet[2]);
//!   spi_write2(packet[3]);
//!   spi_write2(packet[4]);
//!   spi_write2(packet[5]);
   

   return MMCSD_GOODEC;
}


MMCSD_err mmcsd_get_r1(void)
{
   uint8_t
      response = 0,  // place to hold the response coming back from the SPI line
      timeout = 0xFF; // maximum amount loops to wait for idle before getting impatient and leaving the function with an error code
    
   // loop until timeout == 0
   while(timeout)
   {
      // read what's on the SPI line
      //  the SD/MMC requires that you leave the line high when you're waiting for data from it
      response = MMCSD_SPI_XFER(0xFF);
      //response = MMCSD_SPI_XFER(0x00);//leave the line idle
      
      // check to see if we got a response
      if(response != 0xFF)
      {   
         // fill in the response that we got and leave the function
         return response;
      }

      // wait for a little bit longer
      timeout--;
   }
   
   // for some reason, we didn't get a response back from the card
   //  return the proper error codes
   return RESP_TIMEOUT;
}

MMCSD_err mmcsd_get_r2(uint8_t r2[])
{
   r2[1] = mmcsd_get_r1();
   
   r2[0] = MMCSD_SPI_XFER(0xFF);
   
   return 0;
}

MMCSD_err mmcsd_get_r3(uint8_t r3[])
{
   return mmcsd_get_r7(r3);
}

MMCSD_err mmcsd_get_r7(uint8_t r7[])
{
   uint8_t i;   // counter for loop
   
   // the top byte of r7 is r1
   r7[4]=mmcsd_get_r1();
   
   // fill in the other 4 bytes
   for(i = 0; i < 4; i++)
      r7[3 - i] = MMCSD_SPI_XFER(0xFF);

   return r7[4];
}

MMCSD_err mmcsd_go_idle_state(void)
{
   mmcsd_send_cmd(GO_IDLE_STATE, 0, 0);
   
   return mmcsd_get_r1();
}

MMCSD_err mmcsd_send_op_cond(void)
{
   mmcsd_send_cmd(SEND_OP_COND, 0, 0);
   
   return mmcsd_get_r1();
}

MMCSD_err mmcsd_send_if_cond(uint8_t r7[])
{
   mmcsd_send_cmd(SEND_IF_COND, 0x1AA, 0);

   return mmcsd_get_r7(r7);
}

SDRESULTS SD_Init(SD_DEV *dev)
{
#if defined(_M_IX86)    // x86 
    dev->fp = fopen(dev->fn, "r+");
    if (dev->fp == NULL)
        return (SD_ERROR);
    else
    {
        dev->last_sector = __SD_Sectors(dev);
#ifdef SD_IO_DBG_COUNT
        dev->debug.read = 0;
        dev->debug.write = 0;
#endif
        return (SD_OK);
    }
#else   // uControllers
    uint8_t n, cmd, ct, ocr[4];
    uint8_t idx;
	uint8_t r1;
    uint8_t init_trys;
    ct = 0;

	mmcsd_deselect();
	delay_ms(250);
	for(idx = 0; idx < 10; idx++)                        // Send 80 cycles
		MMCSD_SPI_XFER(0xFF);
	/* begin initialization */
	idx = 0;
	do
	{
		delay_ms(1);
		mmcsd_select();
		r1 = mmcsd_go_idle_state();
		mmcsd_deselect();
		idx++;
		if(idx == 0xFF) {
			//if (r1 == 0)
				return SD_NOINIT;
			//else
			//	return r1;
		}
	} while(r1 != MMCSD_IDLE);

   idx = 0;
   do
   {
      delay_ms(1);
      mmcsd_select();
      r1 = mmcsd_send_op_cond();
      mmcsd_deselect();
      idx++;
   } while((r1 & MMCSD_IDLE) && idx != 0xFF);
   if(idx == 0xFF) {
     delay_ms(100);
     mmcsd_select();
     r1 = mmcsd_go_idle_state();
     mmcsd_deselect();
     delay_ms(100);
     mmcsd_select();
     r1 = mmcsd_send_if_cond(ocr);
     mmcsd_deselect();
	if(r1 != MMCSD_IDLE)
		return SD_NOINIT;
   }

	puts("Got thur 872"); crlf();

	return SD_ERROR;


/*
    for(init_trys=0; ((init_trys!=SD_INIT_TRYS)&&(!ct)); init_trys++)
    {
        // Initialize SPI for use with the memory card
        //SPI_Init();

        spi_cs_1();
        //SPI_Freq_Low();

        // 80 dummy clocks
        for(idx = 0; idx != 10; idx++) spi_txrx_byte(0xFF);
        //SPI_Timer_On(500);
        //while(SPI_Timer_Status()==TRUE);
        //SPI_Timer_Off();

        dev->mount = 0;
        //SPI_Timer_On(500);
        do
		{
			dly_us(1000);
			r1 = __SD_Send_Cmd(CMD0, 0, 0);
			spi_cs_1();
		} while (r1 != 1);//&&(SPI_Timer_Status()==TRUE));
		
        //SPI_Timer_Off();
        // Idle state
        //if (__SD_Send_Cmd(CMD0, 0, 0) == 1) {                      
			r1 = __SD_Send_Cmd(CMD8, 0x1AA, 0);
			spi_cs_1();
			puts("OP_COND: ");  serial_hexnum8(r1); crlf();
            // SD version 2?
            if (r1 == 1) {
                // Get trailing return value of R7 resp
                for (n = 0; n < 4; n++) ocr[n] = spi_txrx_byte(0xFF);
                // VDD range of 2.7-3.6V is OK?  
                if ((ocr[2] == 0x01)&&(ocr[3] == 0xAA))
                {
                    // Wait for leaving idle state (ACMD41 with HCS bit)...
                    //SPI_Timer_On(1000);
                    while ((__SD_Send_Cmd(ACMD41, 0, 1U << 14)));//(SPI_Timer_Status()==TRUE)&&
                    //SPI_Timer_Off(); 
                    // CCS in the OCR?
                    if ((__SD_Send_Cmd(CMD58, 0, 0) == 0)) //(SPI_Timer_Status()==TRUE)&&
                    {
                        for (n = 0; n < 4; n++) ocr[n] = spi_txrx_byte(0xFF);
                        // SD version 2?
                        ct = (ocr[0] & 0x40) ? SDCT_SD2 | SDCT_BLOCK : SDCT_SD2;
                    }
                }
            } else {
                // SD version 1 or MMC?
                if (__SD_Send_Cmd(ACMD41, 0, 0) <= 1)
                {
                    // SD version 1
                    ct = SDCT_SD1; 
                    cmd = ACMD41;
                } else {
                    // MMC version 3
                    ct = SDCT_MMC; 
                    cmd = CMD1;
                }
                // Wait for leaving idle state
                //SPI_Timer_On(250);
                while((__SD_Send_Cmd(cmd, 0, 0))); //(SPI_Timer_Status()==TRUE)&&
                //SPI_Timer_Off();
                //if(SPI_Timer_Status()==FALSE) ct = 0;
                if(__SD_Send_Cmd(CMD59, 0, 0))   ct = 0;   // Deactivate CRC check (default)
                if(__SD_Send_Cmd(CMD16, 512, 0)) ct = 0;   // Set R/W block length to 512 bytes
            }
        //}
    }
    if(ct) {
		puts("CT: "); serial_itoa(ct); crlf();
        dev->cardtype = ct;
        dev->mount = 1;
		__SD_Sectors(dev, &dev->last_sector_lo, &dev->last_sector_hi);
		puts("last_lo: "); serial_hexnum16(dev->last_sector_lo); crlf();
		puts("last_hi: "); serial_hexnum16(dev->last_sector_hi); crlf();
        //dev->last_sector = __SD_Sectors(dev) - 1;
#ifdef SD_IO_DBG_COUNT
        dev->debug.read = 0;
        dev->debug.write = 0;
#endif
        //__SD_Speed_Transfer(HIGH); // High speed transfer
    }
	spi_cs_1();
    //SPI_Release();
    return (ct ? SD_OK : SD_NOINIT);*/
#endif
}

SDRESULTS SD_Read(SD_DEV *dev, uint8_t *dat, uint16_t sector_lo, uint16_t sector_hi, uint16_t ofs, uint16_t cnt)
{
#if defined(_M_IX86)    // x86
    // Check the sector query
    if((sector > dev->last_sector)||(cnt == 0)) return(SD_PARERR);
    if(dev->fp!=NULL)
    {
        if (fseek(dev->fp, ((512 * sector) + ofs), SEEK_SET)!=0)
            return(SD_ERROR);
        else {
            if(fread(dat, 1, (cnt - ofs),dev->fp)==(cnt - ofs))
            {
#ifdef SD_IO_DBG_COUNT
                dev->debug.read++;
#endif
                return(SD_OK);
            }
            else return(SD_ERROR);
        }
    } else {
        return(SD_ERROR);
    }
#else   // uControllers
    SDRESULTS res;
    uint8_t tkn;
    uint16_t remaining;
    res = SD_ERROR;
    // TODO: if ((sector > dev->last_sector)||(cnt == 0)) return(SD_PARERR);
    // Convert sector number to byte address (sector * SD_BLK_SIZE)
	uint16_t byte_addr_lo;
	uint16_t byte_addr_hi;
	longxlong(sector_lo, sector_hi, SD_BLK_SIZE, 0, &byte_addr_lo, &byte_addr_hi);
	puts("SD_Read: sec_lo/hi: "); serial_hexnum16(sector_lo); cout(' '); serial_hexnum16(sector_hi); crlf();
	puts("SD_Read: byte_lo/hi: "); serial_hexnum16(byte_addr_lo); cout(' '); serial_hexnum16(byte_addr_hi); crlf();
    if (__SD_Send_Cmd(CMD17, byte_addr_lo, byte_addr_hi) == 0) {
        //SPI_Timer_On(100);  // Wait for data packet (timeout of 100ms)
        do {
            tkn = spi_txrx_byte(0xFF);
        } while((tkn==0xFF));//&&(SPI_Timer_Status()==TRUE));
        //SPI_Timer_Off();
        // Token of single block?
        if(tkn==0xFE) { 
            // Size block (512 bytes) + CRC (2 bytes) - offset - bytes to count
            remaining = SD_BLK_SIZE + 2 - ofs - cnt;
            // Skip offset
            if(ofs) { 
                do { 
                    spi_txrx_byte(0xFF); 
                } while(--ofs);
            }
            // I receive the data and I write in user's buffer
            do {
                tkn = spi_txrx_byte(0xFF);
				//puts("RX "); serial_hexnum16(cnt); cout(' '); serial_hexnum8(tkn); crlf();
				*dat = tkn;
                dat++;
            } while(--cnt);
            // Skip remaining
            do { 
                spi_txrx_byte(0xFF); 
            } while (--remaining);
            res = SD_OK;
        }
    }
	spi_cs_1();
    //SPI_Release();
#ifdef SD_IO_DBG_COUNT
    dev->debug.read++;
#endif
    return(res);
#endif
}

SDRESULTS SD_Status(SD_DEV *dev)
{
#if defined(_M_IX86)
    return((dev->fp == NULL) ? SD_OK : SD_NORESPONSE);
#else
    return(__SD_Send_Cmd(CMD0, 0, 0) ? SD_OK : SD_NORESPONSE);
#endif
}

#endif

void main(void)
{
	uint8_t rx;
	uint8_t counter = 255;
	uint16_t i;
	SD_DEV dev;
	uint8_t buff[32] = { 0 };
	SDRESULTS res;
	uint16_t sector_lo = 0, sector_hi = 0;

	crlf();

	cout('H'); cout('E'); cout('Y');crlf();
	//puts("Hi"); crlf();

	#if 0 //TEST_RODATA 
	puts("Test of .data that was moved into RAM (besides this message): ");
	puts(dstr);
	crlf();	
	
	serial_itoa(1 * sc);crlf();
	serial_itoa(1 * uc);crlf();
	
	puts("And now suck my dick digital 2!\r\n");
	#endif
	
	init_gcs(6, 0x2000, 0x2040);
	set_leds(counter);
	
	init_spi();
	//init_i2c();
	
	while(1)
	{
		if (cin_kbhit())
		{
			rx = cin_buf(); 
			cout(rx);
			
			counter--;
			set_leds(counter);
			
			switch(rx)
			{
				case 'Q':
				{
					asm("jmp 0F000h:0FFF0h");
					break;
				}
				case 's':
				{
					print_bin(get_in_u8());
					break;
				}
				case 'd':
				{
					print_bin(get_in_u4());
					break;
				}
				case 'n':
				{
					sector_lo++;
					puts("sector: "); serial_hexnum16(sector_lo); cout(' '); serial_hexnum16(sector_hi); crlf();
					break;
				}
				case 'p':
				{
					sector_lo--;
					puts("sector: "); serial_hexnum16(sector_lo); cout(' '); serial_hexnum16(sector_hi); crlf();
					break;
				}
				case 'i':
				{
					res = SD_Init(&dev);
					puts("SD_Init: "); serial_hexnum16(res); crlf();
					break;
				}
				case 'r':
				{
					res = SD_Read(&dev, buff, sector_lo, sector_hi, 0, sizeof(buff));
					puts("SD_Read: "); serial_hexnum16(res); crlf();
					serial_hexdump(buff, sizeof(buff));
					break;
				}
				/*case 'i':
				{
					puts("I2C: ");
					if (i2c_send_start(0x50, 0))
					{
						puts("OK");
					}
					else
					{
						puts("NAK");
					}
					crlf();
					i2c_send_stop();
					break;
				}
				case 'j':
				{
					puts("SDA: ");
					if (i2c_sample_sda())
					{
						puts("ON");
					}
					else
					{
						puts("OFF");
					}
					crlf();
					break;
				}
				case 'r':
				{
					puts("CNT: ");
					if (eeprom_24c16_read(0x50, 0, buff, 256))
					{
						puts("OK");crlf();
						for (i = 0; i < 256; i++)
						{
							serial_itoa(buff[i]); cout(' ');
							if (i % 16 == 15)
							{
								crlf();
							}
						}
					}
					else
					{
						puts("ERR");
					}
					crlf();
					break;
				}*/
			}
		}
		
		/*if (get_in_u8() & 0x8)
		{
			cout('S'); cout('W'); cout('2'); crlf();
			while(get_in_u8() & 0x8);
		}*/	
	}
}

