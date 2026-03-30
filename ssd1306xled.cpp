/**
 * @file ssd1306xled.cpp
 * @brief SSD1306 OLED driver implementation.
 *
 * I2C is bit-banged through the ATtiny85 USI peripheral rather than using
 * the Wire library. This saves about 1 KB of flash. The implementation is
 * based on TinyI2C by David Johnson-Davies.
 *
 * Display data goes straight to the SSD1306 over I2C -- there is no local
 * framebuffer. This keeps RAM usage near zero but means every draw call is an
 * I2C transaction, and the display cannot be read back (no read-modify-write).
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

/** @cond INTERNAL */

// Horizontal addressing mode init sequence.
const uint8_t ssd1306_init_sequence [] PROGMEM = {
	0xAE,			// Display OFF
	0xD5, 0xF0,		// Clock divide ratio / oscillator frequency
	0xA8, 0x3F,		// Multiplex ratio: 64 rows (0x3F = 63)
	0xD3, 0x00,		// Display offset: none
	0x40 | 0x00,	// Start line: row 0
	0x8D, 0x14,		// Charge pump: enable
	0x20, 0x00,		// Addressing mode: horizontal
	0xA0 | 0x01,	// Segment remap: column 127 = SEG0
	0xC8,			// COM scan: bottom to top
	0xDA, 0x12,		// COM pins: 128x64 config
	0x81, 0x3F,		// Contrast: max
	0xD9, 0x22,		// Pre-charge period
	0xDB, 0x20,		// VCOMH deselect: 0.77 x VCC
	0xA4,			// Display from RAM (not all-on test mode)
	0xA6,			// Normal display (not inverted)
	0x2E,			// Deactivate scroll
	0xAF,			// Display ON
};

// Vertical addressing mode init sequence.
const uint8_t ssd1306_init_sequence_vertical_addressing_mode [] PROGMEM = {
	0xAE,
	0xD5, 0xF0,
	0xA8, 0x3F,
	0xD3, 0x00,
	0x40 | 0x00,
	0x8D, 0x14,
	0x20, 0x01,		// Addressing mode: vertical
	0x21, 0x00, 0x7f,	// Column address range: 0-127
	0xA0 | 0x01,
	0xC8,
	0xDA, 0x12,
	0x81, 0x3F,
	0xD9, 0x22,
	0xDB, 0x20,
	0xA4,
	0xA6,
	0x2E,
	0xAF,
};
/** @endcond */

SSD1306Device::SSD1306Device(void){}

// ----------------------------------------------------------------------------
// I2C bit-banging via USI
// ----------------------------------------------------------------------------

/**
 * @internal
 * Configure USI for two-wire (I2C) mode. Enables pull-ups on SDA and SCL,
 * sets both as outputs, and preloads the data register with 0xFF (idle state).
 */
void SSD1306Device::I2CInit() {
	PORT_USI |= 1<<PIN_USI_SDA;
	PORT_USI_CL |= 1<<PIN_USI_SCL;

	DDR_USI_CL |= 1<<PIN_USI_SCL;
	DDR_USI |= 1<<PIN_USI_SDA;

	USIDR = 0xFF;
	USICR = 0<<USISIE | 0<<USIOIE |
			1<<USIWM1 | 0<<USIWM0 |
			1<<USICS1 | 0<<USICS0 | 1<<USICLK |
			0<<USITC;
	USISR = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC |
			0x0<<USICNT0;
}

/**
 * @internal
 * Send I2C START condition and the slave address (write mode).
 * @param address 7-bit I2C address (shifted left internally).
 * @return true if the slave ACK'd, false on timeout or NACK.
 */
bool SSD1306Device::I2CStart(uint8_t address) {
	uint8_t addressRW = address << 1;

	// Release SCL so a (repeated) START can be performed
	PORT_USI_CL |= 1<<PIN_USI_SCL;
	while (!(PIN_USI_CL & 1<<PIN_USI_SCL));
	#ifdef TWI_FAST_MODE
	DELAY_T4TWI;
	#else
	DELAY_T2TWI;
	#endif

	// Generate START: SDA goes low while SCL is high
	PORT_USI &= ~(1<<PIN_USI_SDA);
	DELAY_T4TWI;
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);
	PORT_USI |= 1<<PIN_USI_SDA;

	if (!(USISR & 1<<USISIF)) return false;

	// Send address byte
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);
	USIDR = addressRW;
	I2CTransfer(USISR_8bit);

	// Check ACK
	DDR_USI &= ~(1<<PIN_USI_SDA);
	if (I2CTransfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

	return true;
}

/**
 * @internal
 * Clock bits through USI. The data parameter is loaded into USISR to control
 * how many bits to shift (8 for data, 1 for ACK).
 */
uint8_t SSD1306Device::I2CTransfer (uint8_t data) {
  USISR = data;
  data = 0<<USISIE | 0<<USIOIE |
         1<<USIWM1 | 0<<USIWM0 |
         1<<USICS1 | 0<<USICS0 | 1<<USICLK |
         1<<USITC;
  do {
    DELAY_T2TWI;
    USICR = data;                             // Positive SCL edge
    while (!(PIN_USI_CL & 1<<PIN_USI_SCL));   // Wait for SCL high
    DELAY_T4TWI;
    USICR = data;                             // Negative SCL edge
  } while (!(USISR & 1<<USIOIF));

  DELAY_T2TWI;
  data = USIDR;
  USIDR = 0xFF;                               // Release SDA
  DDR_USI |= (1<<PIN_USI_SDA);

  return data;
}

/** @internal Generate I2C STOP condition. */
void SSD1306Device::I2CStop (void) {
  PORT_USI &= ~(1<<PIN_USI_SDA);              // Pull SDA low
  PORT_USI_CL |= 1<<PIN_USI_SCL;              // Release SCL
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));
  DELAY_T4TWI;
  PORT_USI |= 1<<PIN_USI_SDA;                 // Release SDA
  DELAY_T2TWI;
}

/**
 * @internal
 * Initialize I2C and verify the display is reachable by retrying I2CStart
 * until it ACKs. Skipped if SSD1306_QUICK_BEGIN is defined.
 */
void SSD1306Device::begin() {
	I2CInit();
#ifndef SSD1306_QUICK_BEGIN
	while (!I2CStart(SSD1306_SA)) {
		delay(10);
	}
	I2CStop();
#endif
}

/**
 * @internal
 * Write one byte to the I2C bus and check for ACK.
 * @return true if slave ACK'd.
 */
bool SSD1306Device::I2CWrite(uint8_t data)  {
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);
  USIDR = data;
  I2CTransfer(USISR_8bit);

  DDR_USI &= ~(1<<PIN_USI_SDA);
  if (I2CTransfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

  return true;
}

// --- Shared internal helpers ---

/**
 * @internal
 * Start an I2C transaction to the display in the given mode (command or data).
 * Stops any previous transaction first.
 */
void SSD1306Device::_ssd1306_start(uint8_t mode) {
	I2CStop();
	I2CStart(SSD1306_SA);
	I2CWrite(mode);
}

/**
 * @internal
 * Send an init sequence from PROGMEM. Used by all three init functions to
 * avoid duplicating the command-send loop.
 */
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
	ssd1306_send_data_start();
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
	ssd1306_send_data_stop();
}

void SSD1306Device::ssd1306_setpos(uint8_t x, uint8_t y)
{
	ssd1306_send_command_start();
	ssd1306_send_byte(0xb0 | (y & 0x07));         // Page address
	ssd1306_send_byte(0x10 | ((x & 0xf0) >> 4));  // Column high nibble
	ssd1306_send_byte(x & 0x0f);                   // Column low nibble
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


// Pixel-level bitmap drawing.
// The bit-shift algorithm: for a sprite at y_px that doesn't land on a page
// boundary (offset != 0), each display page gets bits from two adjacent bitmap
// rows. The previous row's bits shift right by (8 - offset) into the upper
// portion, and the current row's bits shift left by offset into the lower
// portion, then OR together. This lets a sprite sit at any vertical pixel
// position without needing a framebuffer.
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

	if (x >= 128 || x + (int16_t)w <= 0) return;

	if (x < 0) {
		col_offset = (uint8_t)(-x);
		w -= col_offset;
		x = 0;
	}

	// orig_w: the real bitmap row stride (before right-side clipping)
	uint8_t orig_w = w + col_offset;

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
	if (x >= 128 || x + (int16_t)w <= 0) return;

	if (x < 0) {
		uint8_t clip = (uint8_t)(-x);
		w -= clip;
		x = 0;
	}

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

	if (target_page < page_start || target_page >= page_start + total_pages) return;

	uint8_t rp = target_page - page_start;

	for (uint8_t c = 0; c < sprite_w; c++)
	{
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
