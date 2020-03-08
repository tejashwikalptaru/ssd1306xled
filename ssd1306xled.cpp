/*
 * SSD1306xLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x64 displays
 *
 * @created: 2014-08-12
 * @author: Neven Boyanov
 *
 * Source code available at: https://bitbucket.org/tinusaur/ssd1306xled
 * 
 * Modified by Tejashwi Kalp Taru, with the help of TinyI2C (https://github.com/technoblogy/tiny-i2c/)
 * Modified code available at: https://github.com/tejashwikalptaru/ssd1306xled
 */

// ----------------------------------------------------------------------------


#include <stdlib.h>
#include <avr/io.h>

#include <avr/pgmspace.h>

#include "ssd1306xled.h"
#include "font6x8.h"
#include "font8x16.h"

// ----------------------------------------------------------------------------

// Some code based on "IIC_wtihout_ACK" by http://www.14blog.com/archives/1358

const uint8_t ssd1306_init_sequence [] PROGMEM = {	// Initialization Sequence
	0xAE,			// Set Display ON/OFF - AE=OFF, AF=ON
	0xD5, 0xF0,		// Set display clock divide ratio/oscillator frequency, set divide ratio
	0xA8, 0x3F,		// Set multiplex ratio (1 to 64) ... (height - 1)
	0xD3, 0x00,		// Set display offset. 00 = no offset
	0x40 | 0x00,	// Set start line address, at 0.
	0x8D, 0x14,		// Charge Pump Setting, 14h = Enable Charge Pump
	0x20, 0x00,		// Set Memory Addressing Mode - 00=Horizontal, 01=Vertical, 10=Page, 11=Invalid
	0xA0 | 0x01,	// Set Segment Re-map
	0xC8,			// Set COM Output Scan Direction
	0xDA, 0x12,		// Set COM Pins Hardware Configuration - 128x32:0x02, 128x64:0x12
	0x81, 0x3F,		// Set contrast control register
	0xD9, 0x22,		// Set pre-charge period (0x22 or 0xF1)
	0xDB, 0x20,		// Set Vcomh Deselect Level - 0x00: 0.65 x VCC, 0x20: 0.77 x VCC (RESET), 0x30: 0.83 x VCC
	0xA4,			// Entire Display ON (resume) - output RAM to display
	0xA6,			// Set Normal/Inverse Display mode. A6=Normal; A7=Inverse
	0x2E,			// Deactivate Scroll command
	0xAF,			// Set Display ON/OFF - AE=OFF, AF=ON
};

// Program:    5248 bytes

SSD1306Device::SSD1306Device(void){}

void SSD1306Device::I2CInit() {
	PORT_USI |= 1<<PIN_USI_SDA;                 // Enable pullup on SDA.
	PORT_USI_CL |= 1<<PIN_USI_SCL;              // Enable pullup on SCL.

	DDR_USI_CL |= 1<<PIN_USI_SCL;               // Enable SCL as output.
	DDR_USI |= 1<<PIN_USI_SDA;                  // Enable SDA as output.

	USIDR = 0xFF;                               // Preload data register with "released level" data.
	USICR = 0<<USISIE | 0<<USIOIE |             // Disable Interrupts.
			1<<USIWM1 | 0<<USIWM0 |             // Set USI in Two-wire mode.
			1<<USICS1 | 0<<USICS0 | 1<<USICLK | // Software stobe as counter clock source
			0<<USITC;
	USISR = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | // Clear flags,
			0x0<<USICNT0;                       // and reset counter.
}

bool SSD1306Device::I2CStart(uint8_t address, int readcount) {
	if (readcount != 0) { I2Ccount = readcount; readcount = 1; }
	uint8_t addressRW = address<<1 | readcount;

	/* Release SCL to ensure that (repeated) Start can be performed */
	PORT_USI_CL |= 1<<PIN_USI_SCL;              // Release SCL.
	while (!(PIN_USI_CL & 1<<PIN_USI_SCL));     // Verify that SCL becomes high.
	#ifdef TWI_FAST_MODE
	DELAY_T4TWI;
	#else
	DELAY_T2TWI;
	#endif

	/* Generate Start Condition */
	PORT_USI &= ~(1<<PIN_USI_SDA);              // Force SDA LOW.
	DELAY_T4TWI;
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);           // Pull SCL LOW.
	PORT_USI |= 1<<PIN_USI_SDA;                 // Release SDA.

	if (!(USISR & 1<<USISIF)) return false;

	/*Write address */
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);           // Pull SCL LOW.
	USIDR = addressRW;                          // Setup data.
	I2CTransfer(USISR_8bit);                 // Send 8 bits on bus.

	/* Clock and verify (N)ACK from slave */
	DDR_USI &= ~(1<<PIN_USI_SDA);               // Enable SDA as input.
	if (I2CTransfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false; // No ACK

	return true;                                // Start successfully completed
}

uint8_t SSD1306Device::I2CTransfer (uint8_t data) {
  USISR = data;                               // Set USISR according to data.
                                              // Prepare clocking.
  data = 0<<USISIE | 0<<USIOIE |              // Interrupts disabled
         1<<USIWM1 | 0<<USIWM0 |              // Set USI in Two-wire mode.
         1<<USICS1 | 0<<USICS0 | 1<<USICLK |  // Software clock strobe as source.
         1<<USITC;                            // Toggle Clock Port.
  do {
    DELAY_T2TWI;
    USICR = data;                             // Generate positive SCL edge.
    while (!(PIN_USI_CL & 1<<PIN_USI_SCL));   // Wait for SCL to go high.
    DELAY_T4TWI;
    USICR = data;                             // Generate negative SCL edge.
  } while (!(USISR & 1<<USIOIF));             // Check for transfer complete.

  DELAY_T2TWI;
  data = USIDR;                               // Read out data.
  USIDR = 0xFF;                               // Release SDA.
  DDR_USI |= (1<<PIN_USI_SDA);                // Enable SDA as output.

  return data;                                // Return the data from the USIDR
}

void SSD1306Device::I2CStop (void) {
  PORT_USI &= ~(1<<PIN_USI_SDA);              // Pull SDA low.
  PORT_USI_CL |= 1<<PIN_USI_SCL;              // Release SCL.
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));     // Wait for SCL to go high.
  DELAY_T4TWI;
  PORT_USI |= 1<<PIN_USI_SDA;                 // Release SDA.
  DELAY_T2TWI;
}

void SSD1306Device::begin() {
	I2CInit();
#ifndef TINY4KOLED_QUICK_BEGIN
	while (!I2CStart(SSD1306_SA, 0)) {
		delay(10);
	}
	I2CStop();
#endif
}

bool SSD1306Device::I2CWrite(uint8_t data)  {
	/* Write a byte */
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);           // Pull SCL LOW.
  USIDR = data;                               // Setup data.
  I2CTransfer(USISR_8bit);                 // Send 8 bits on bus.

  /* Clock and verify (N)ACK from slave */
  DDR_USI &= ~(1<<PIN_USI_SDA);               // Enable SDA as input.
  if (I2CTransfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

  return true;
}


void SSD1306Device::ssd1306_init(void)
{
	begin();
	ssd1306_send_command_start();
	for (uint8_t i = 0; i < sizeof (ssd1306_init_sequence); i++) {
		ssd1306_send_byte(pgm_read_byte(&ssd1306_init_sequence[i]));
	}
	ssd1306_send_command_stop();
	ssd1306_fillscreen(0);
}

void SSD1306Device::ssd1306_send_command_start(void) {
	I2CStop();
	I2CStart(SSD1306_SA, 0);
	I2CWrite(SSD1306_COMMAND);
}

void SSD1306Device::ssd1306_send_command_stop() {
	I2CStop();
}

void SSD1306Device::ssd1306_send_command(uint8_t command) {
	ssd1306_send_command_start();
	ssd1306_send_byte(command);
	ssd1306_send_command_stop();
}

void SSD1306Device::ssd1306_send_byte(uint8_t byte) {
	I2CWrite(byte);
}

void SSD1306Device::ssd1306_send_data_start(void) {
	I2CStop();
	I2CStart(SSD1306_SA, 0);
	I2CWrite(SSD1306_DATA);
}

void SSD1306Device::ssd1306_send_data_stop() {
	I2CStop();
}

void SSD1306Device::ssd1306_fillscreen(uint8_t fill) {
	ssd1306_setpos(0, 0);
	ssd1306_send_data_start();	// Initiate transmission of data
	for (uint16_t i = 0; i < 128 * 8 / 4; i++) {
		ssd1306_send_byte(fill);
		ssd1306_send_byte(fill);
		ssd1306_send_byte(fill);
		ssd1306_send_byte(fill);
	}
	ssd1306_send_data_stop();	// Finish transmission
}

void SSD1306Device::ssd1306_setpos(uint8_t x, uint8_t y)
{
	ssd1306_send_command_start();
	ssd1306_send_byte(0xb0 | (y & 0x07));
	ssd1306_send_byte(0x10 | ((x & 0xf0) >> 4));
	ssd1306_send_byte(x & 0x0f); // | 0x01
	ssd1306_send_command_stop();
}

void SSD1306Device::ssd1306_char_font6x8(char ch) {
	uint8_t i; 
	uint8_t c = ch - 32;
	ssd1306_send_data_start();
	for (i= 0; i < 6; i++)
	{
		ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font6x8[c * 6 + i]));
	}
	ssd1306_send_data_stop();
}

void SSD1306Device::ssd1306_string_font6x8(char *s) {
	while (*s) {
		ssd1306_char_font6x8(*s++);
	}
}

void SSD1306Device::ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]){
	uint16_t j = 0;
	uint8_t y, x;
	if (y1 % 8 == 0) y = y1 / 8;
	else y = y1 / 8 + 1;
	for (y = y0; y < y1; y++)
	{
		ssd1306_setpos(x0,y);
		ssd1306_send_data_start();
		for (x = x0; x < x1; x++)
		{
			ssd1306_send_byte(pgm_read_byte(&bitmap[j++]));
		}
		ssd1306_send_data_stop();
	}
}

void SSD1306Device::ssd1306_char_f8x16(uint8_t x, uint8_t y, const char ch[])
{
	uint8_t c, j, i = 0;
	while (ch[j] != '\0')
	{
		c = ch[j] - 32;
		if (x > 120)
		{
			x = 0;
			y++;
		}
		ssd1306_setpos(x, y);
		ssd1306_send_data_start();
		for (i = 0; i < 8; i++)
		{
			ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font8x16[c * 16 + i]));
		}
		ssd1306_send_data_stop();
		ssd1306_setpos(x, y + 1);
		ssd1306_send_data_start();
		for (i = 0; i < 8; i++)
		{
			ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font8x16[c * 16 + i + 8]));
		}
		ssd1306_send_data_stop();
		x += 8;
		j++;
	}
}



SSD1306Device SSD1306;

// ----------------------------------------------------------------------------