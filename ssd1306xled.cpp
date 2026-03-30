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

#ifndef SSD1306_NO_FONT_6X8
#include "font6x8.h"
#endif

#ifndef SSD1306_NO_FONT_8X16
#include "font8x16.h"
#endif

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

const uint8_t ssd1306_init_sequence_vertical_addressing_mode [] PROGMEM = {	// Initialization Sequence for Vertical Addressing Mode
	0xAE,			// Set Display ON/OFF - AE=OFF, AF=ON
	0xD5, 0xF0,		// Set display clock divide ratio/oscillator frequency, set divide ratio
	0xA8, 0x3F,		// Set multiplex ratio (1 to 64) ... (height - 1)
	0xD3, 0x00,		// Set display offset. 00 = no offset
	0x40 | 0x00,	// Set start line address, at 0.
	0x8D, 0x14,		// Charge Pump Setting, 14h = Enable Charge Pump
	0x20, 0x01,		// Set Memory Addressing Mode - 00=Horizontal, 01=Vertical, 10=Page, 11=Invalid
	0x21, 0x00, 0x7f,		// Set Column Address - start address (0x00), end address (0x7f)
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

bool SSD1306Device::I2CStart(uint8_t address) {
	uint8_t addressRW = address << 1;

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
	while (!I2CStart(SSD1306_SA)) {
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

// --- Shared internal helpers ---

void SSD1306Device::_ssd1306_start(uint8_t mode) {
	I2CStop();
	I2CStart(SSD1306_SA);
	I2CWrite(mode);
}

void SSD1306Device::_ssd1306_init_from(const uint8_t *seq, uint8_t len) {
	begin();
	ssd1306_send_command_start();
	for (uint8_t i = 0; i < len; i++) {
		ssd1306_send_byte(pgm_read_byte(&seq[i]));
	}
	ssd1306_send_command_stop();
}

// --- Public API ---

void SSD1306Device::ssd1306_init(void)
{
	_ssd1306_init_from(ssd1306_init_sequence, sizeof(ssd1306_init_sequence));
	ssd1306_fillscreen(0);
}

void SSD1306Device::ssd1306_tiny_init(void)
{
	_ssd1306_init_from(ssd1306_init_sequence, sizeof(ssd1306_init_sequence));
}

void SSD1306Device::ssd1306_tiny_init_vertical(void)
{
	_ssd1306_init_from(ssd1306_init_sequence_vertical_addressing_mode, sizeof(ssd1306_init_sequence_vertical_addressing_mode));
}

void SSD1306Device::ssd1306_send_command_start(void) {
	_ssd1306_start(SSD1306_COMMAND);
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
	_ssd1306_start(SSD1306_DATA);
}

void SSD1306Device::ssd1306_send_data_stop() {
	I2CStop();
}

void SSD1306Device::ssd1306_fillscreen(uint8_t fill) {
	ssd1306_setpos(0, 0);
	ssd1306_send_data_start();	// Initiate transmission of data
#ifdef SSD1306_FAST_FILLSCREEN
	for (uint16_t i = 0; i < 128 * 8 / 4; i++) {
		ssd1306_send_byte(fill);
		ssd1306_send_byte(fill);
		ssd1306_send_byte(fill);
		ssd1306_send_byte(fill);
	}
#else
	for (uint16_t i = 0; i < 1024; i++) {
		ssd1306_send_byte(fill);
	}
#endif
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

#ifndef SSD1306_NO_FONT_6X8

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
	ssd1306_send_data_start();
	while (*s) {
		uint8_t c = *s++ - 32;
		for (uint8_t i = 0; i < 6; i++)
		{
			ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font6x8[c * 6 + i]));
		}
	}
	ssd1306_send_data_stop();
}

#endif // SSD1306_NO_FONT_6X8

#ifndef SSD1306_NO_DRAW_BMP

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

#endif // SSD1306_NO_DRAW_BMP

#ifndef SSD1306_NO_FONT_8X16

void SSD1306Device::ssd1306_char_f8x16(uint8_t x, uint8_t y, const char ch[])
{
	uint8_t c, j = 0, i = 0;
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

#endif // SSD1306_NO_FONT_8X16


// Draw bitmap at pixel-level y position (not page-aligned).
// y_px is in pixels (0-63). The sprite is bit-shifted across page boundaries.
// Note: overwrites all 8 vertical pixels in each affected page row — other
// content sharing those page strips will be erased. SSD1306 I2C does not
// support read-modify-write, so existing page content cannot be preserved.
void SSD1306Device::ssd1306_draw_bmp_px(uint8_t x, uint8_t y_px, uint8_t w, uint8_t h_pages, const uint8_t bitmap[])
{
	if (x >= 128) return;
	uint8_t draw_w = w;
	uint8_t max_w = 128 - x;
	if (draw_w > max_w) draw_w = max_w;

	uint8_t page_start = y_px >> 3;
	uint8_t offset = y_px & 0x07;
	uint8_t total_pages = h_pages + (offset ? 1 : 0);

	for (uint8_t rp = 0; rp < total_pages; rp++)
	{
		uint8_t dp = page_start + rp;
		if (dp > 7) break;

		ssd1306_setpos(x, dp);
		ssd1306_send_data_start();

		for (uint8_t c = 0; c < draw_w; c++)
		{
			uint8_t out = 0;

			if (offset == 0)
			{
				out = pgm_read_byte(&bitmap[rp * w + c]);
			}
			else
			{
				if (rp > 0)
					out = pgm_read_byte(&bitmap[(rp - 1) * w + c]) >> (8 - offset);
				if (rp < h_pages)
					out |= pgm_read_byte(&bitmap[rp * w + c]) << offset;
			}

			ssd1306_send_byte(out);
		}
		ssd1306_send_data_stop();
	}
}

// Clear the area that a pixel-positioned sprite occupies.
// Writes zeros to all pages the sprite would touch at the given y_px.
void SSD1306Device::ssd1306_clear_area_px(uint8_t x, uint8_t y_px, uint8_t w, uint8_t h_pages)
{
	if (x >= 128) return;
	uint8_t max_w = 128 - x;
	if (w > max_w) w = max_w;

	uint8_t page_start = y_px >> 3;
	uint8_t offset = y_px & 0x07;
	uint8_t total_pages = h_pages + (offset ? 1 : 0);

	for (uint8_t rp = 0; rp < total_pages; rp++)
	{
		uint8_t dp = page_start + rp;
		if (dp > 7) break;
		ssd1306_setpos(x, dp);
		ssd1306_send_data_start();
		for (uint8_t c = 0; c < w; c++)
			ssd1306_send_byte(0x00);
		ssd1306_send_data_stop();
	}
}

// --- Optional: Signed X clipping support ---
#ifdef SSD1306_CLIPPING

void SSD1306Device::ssd1306_draw_bmp_px_clipped(int16_t x, uint8_t y_px, uint8_t w, uint8_t h_pages, const uint8_t bitmap[])
{
	uint8_t col_offset = 0;

	// Fully off-screen
	if (x >= 128 || x + (int16_t)w <= 0) return;

	// Clip left edge
	if (x < 0) {
		col_offset = (uint8_t)(-x);
		w -= col_offset;
		x = 0;
	}

	// orig_w is the true bitmap row stride (before right-clipping)
	uint8_t orig_w = w + col_offset;

	// Clip right edge
	if (x + w > 128) w = 128 - (uint8_t)x;

	uint8_t ux = (uint8_t)x;
	uint8_t page_start = y_px >> 3;
	uint8_t offset = y_px & 0x07;
	uint8_t total_pages = h_pages + (offset ? 1 : 0);

	for (uint8_t rp = 0; rp < total_pages; rp++)
	{
		uint8_t dp = page_start + rp;
		if (dp > 7) break;

		ssd1306_setpos(ux, dp);
		ssd1306_send_data_start();

		for (uint8_t c = 0; c < w; c++)
		{
			uint8_t sc = c + col_offset;
			uint8_t out = 0;

			if (offset == 0)
			{
				out = pgm_read_byte(&bitmap[rp * orig_w + sc]);
			}
			else
			{
				if (rp > 0)
					out = pgm_read_byte(&bitmap[(rp - 1) * orig_w + sc]) >> (8 - offset);
				if (rp < h_pages)
					out |= pgm_read_byte(&bitmap[rp * orig_w + sc]) << offset;
			}

			ssd1306_send_byte(out);
		}
		ssd1306_send_data_stop();
	}
}

void SSD1306Device::ssd1306_clear_area_px_clipped(int16_t x, uint8_t y_px, uint8_t w, uint8_t h_pages)
{
	// Fully off-screen
	if (x >= 128 || x + (int16_t)w <= 0) return;

	// Clip left edge
	if (x < 0) {
		uint8_t clip = (uint8_t)(-x);
		w -= clip;
		x = 0;
	}

	// Clip right edge
	if (x + w > 128) w = 128 - (uint8_t)x;

	uint8_t ux = (uint8_t)x;
	uint8_t page_start = y_px >> 3;
	uint8_t offset = y_px & 0x07;
	uint8_t total_pages = h_pages + (offset ? 1 : 0);

	for (uint8_t rp = 0; rp < total_pages; rp++)
	{
		uint8_t dp = page_start + rp;
		if (dp > 7) break;
		ssd1306_setpos(ux, dp);
		ssd1306_send_data_start();
		for (uint8_t c = 0; c < w; c++)
			ssd1306_send_byte(0x00);
		ssd1306_send_data_stop();
	}
}

#endif // SSD1306_CLIPPING


// --- Optional: Page compositing support ---
#ifdef SSD1306_COMPOSITING

void SSD1306Device::ssd1306_compose_bmp_px(uint8_t *buf, uint8_t buf_x, uint8_t buf_w,
	int16_t sprite_x, uint8_t sprite_y_px,
	uint8_t sprite_w, uint8_t sprite_h_pages,
	const uint8_t bitmap[], uint8_t target_page)
{
	uint8_t page_start = sprite_y_px >> 3;
	uint8_t offset = sprite_y_px & 0x07;
	uint8_t total_pages = sprite_h_pages + (offset ? 1 : 0);

	// Check if this sprite touches the target page at all
	if (target_page < page_start || target_page >= page_start + total_pages) return;

	uint8_t rp = target_page - page_start;

	for (uint8_t c = 0; c < sprite_w; c++)
	{
		// Map sprite column to buffer position
		int16_t disp_col = sprite_x + c;
		if (disp_col < (int16_t)buf_x || disp_col >= (int16_t)(buf_x + buf_w)) continue;
		if (disp_col < 0 || disp_col >= 128) continue;
		uint8_t buf_idx = (uint8_t)disp_col - buf_x;

		uint8_t out = 0;

		if (offset == 0)
		{
			out = pgm_read_byte(&bitmap[rp * sprite_w + c]);
		}
		else
		{
			if (rp > 0)
				out = pgm_read_byte(&bitmap[(rp - 1) * sprite_w + c]) >> (8 - offset);
			if (rp < sprite_h_pages)
				out |= pgm_read_byte(&bitmap[rp * sprite_w + c]) << offset;
		}

		buf[buf_idx] |= out;
	}
}

void SSD1306Device::ssd1306_send_buf(uint8_t x, uint8_t page, const uint8_t *buf, uint8_t w)
{
	if (x >= 128) return;
	uint8_t max_w = 128 - x;
	if (w > max_w) w = max_w;

	ssd1306_setpos(x, page);
	ssd1306_send_data_start();
	for (uint8_t c = 0; c < w; c++)
		ssd1306_send_byte(buf[c]);
	ssd1306_send_data_stop();
}

#endif // SSD1306_COMPOSITING


SSD1306Device SSD1306;

// ----------------------------------------------------------------------------
