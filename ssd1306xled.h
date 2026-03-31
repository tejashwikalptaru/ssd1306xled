/**
 * @file ssd1306xled.h
 * @brief Driver for SSD1306, SSD1315, and SSH1106 OLED displays.
 *
 * Bit-bangs I2C via the ATtiny85 USI peripheral. No Wire library needed --
 * saves about 1 KB of flash compared to TinyWireM. Works on any AVR with USI
 * by redefining the pin macros.
 *
 * Originally by Neven Boyanov (Tinusaur). I2C rewrite based on TinyI2C by
 * David Johnson-Davies. Maintained by Tejashwi Kalp Taru.
 *
 * @version 1.0.0
 * @see https://github.com/tejashwikalptaru/ssd1306xled
 */

#include <stdint.h>
#include <Arduino.h>
#include <util/delay.h>

#ifndef SSD1306XLED_H
#define SSD1306XLED_H

#define _SSD1306XLED_TINY_INIT_SUPPORTED_
#define _SSD1306XLED_INIT_VERTICAL_SUPPORTED_

// ----------------------------------------------------------------------------
// Hardware pin configuration
// ----------------------------------------------------------------------------

/**
 * @defgroup hw_config Hardware configuration
 * Override these before including the header to change pin assignments
 * or I2C address. Defaults target ATtiny85 with the standard OLED
 * breakout wiring (SCL=PB2, SDA=PB0, address 0x3C).
 * @{
 */

/** @def SSD1306_SCL
 *  I2C clock pin. Default: PB2 (ATtiny85 pin 7). */
#ifndef SSD1306_SCL
#define SSD1306_SCL		PB2
#endif

/** @def SSD1306_SDA
 *  I2C data pin. Default: PB0 (ATtiny85 pin 5). */
#ifndef SSD1306_SDA
#define SSD1306_SDA		PB0
#endif

/** @def SSD1306_SA
 *  7-bit I2C slave address. Default: 0x3C. Some modules use 0x3D. */
#ifndef SSD1306_SA
#define SSD1306_SA		0X3C
#endif

/** @} */ // end hw_config

// USI register mappings (ATtiny85-specific)
#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PORTB0
#define PORT_USI_SCL        PORTB2
#define PIN_USI_SDA         PINB0
#define PIN_USI_SCL         PINB2
#define DDR_USI_CL 			DDR_USI
#define PORT_USI_CL 		PORT_USI
#define PIN_USI_CL 			PIN_USI

// I2C speed mode
#define TWI_FAST_MODE

#ifdef TWI_FAST_MODE                 // TWI FAST mode timing limits. SCL = 100-400kHz
#define DELAY_T2TWI (_delay_us(2))   // >1.3us
#define DELAY_T4TWI (_delay_us(1))   // >0.6us
#else                                // TWI STANDARD mode timing limits. SCL <= 100kHz
#define DELAY_T2TWI (_delay_us(5))   // >4.7us
#define DELAY_T4TWI (_delay_us(4))   // >4.0us
#endif

#define TWI_NACK_BIT 0 // Bit position for (N)ACK bit.

// USI shift register preloads
const unsigned char USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;
const unsigned char USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;

/** I2C control byte: next bytes are commands. */
#define SSD1306_COMMAND 0x00
/** I2C control byte: next bytes are display data. */
#define SSD1306_DATA 0x40

// ----------------------------------------------------------------------------
// Opt-out feature flags
// ----------------------------------------------------------------------------

/**
 * @defgroup build_flags Build flags (PlatformIO)
 * These flags control compilation when passed via PlatformIO `build_flags`
 * (e.g. `-D SSD1306_NO_FONT_6X8` in platformio.ini). They do NOT work as
 * `#define` in Arduino IDE sketches because Arduino compiles library .cpp
 * files separately.
 *
 * In Arduino IDE, the linker already strips unused functions automatically.
 * @{
 */

#ifdef DOXYGEN
/** @def SSD1306_NO_FONT_6X8
 * Exclude the 6x8 font data, ssd1306_char_font6x8(), and
 * ssd1306_string_font6x8(). The 6x8 font costs 678 bytes when used. */
#define SSD1306_NO_FONT_6X8

/** @def SSD1306_NO_FONT_8X16
 * Exclude the 8x16 font data and ssd1306_string_f8x16().
 * The 8x16 font costs 1722 bytes when used. */
#define SSD1306_NO_FONT_8X16

/** @def SSD1306_NO_DRAW_BMP
 * Exclude ssd1306_draw_bmp() (the page-aligned bitmap draw).
 * Use ssd1306_draw_bmp_px() instead. Costs 66 bytes when used. */
#define SSD1306_NO_DRAW_BMP

/** @def SSD1306_QUICK_BEGIN
 * Skip the I2C device-present check during initialization. The normal begin()
 * retries I2CStart until the display ACKs. With this flag, it assumes the
 * display is already there. Saves 62 bytes. */
#define SSD1306_QUICK_BEGIN
#endif

/** @} */ // end opt_out

// ----------------------------------------------------------------------------

/**
 * @class SSD1306Device
 * @brief Driver for SSD1306-family OLED displays over bit-banged I2C.
 *
 * Talks to the display through the ATtiny85 USI peripheral. All drawing goes
 * straight to the display -- there is no framebuffer in RAM (the ATtiny85 only
 * has 512 bytes). The display's 128x64 pixels are organized as 8 pages of 128
 * bytes each. Each byte controls 8 vertical pixels (LSB = top).
 *
 * A global instance @ref SSD1306 is created automatically. Use that instead of
 * making your own.
 */
class SSD1306Device
{
    public:
		SSD1306Device(void);

		/**
		 * @brief Initialize the display and clear the screen.
		 *
		 * Sends the full init sequence (clock, multiplex, charge pump, addressing
		 * mode, etc.) then fills every pixel with zero. Takes about 20 I2C
		 * command bytes (26 total) plus 1024 data bytes.
		 */
		void ssd1306_init(void);

		/**
		 * @brief Initialize the display without clearing the screen.
		 *
		 * Same as ssd1306_init() but skips the fillscreen(0) call. Use this
		 * when you plan to draw immediately and do not care about leftover
		 * content from a previous power cycle. Saves ~52 bytes of flash and
		 * some startup time.
		 */
		void ssd1306_tiny_init(void);

		/**
		 * @brief Initialize the display in vertical addressing mode.
		 *
		 * After this, bytes auto-increment down pages within a column rather
		 * than across columns within a page. Only the low-level data/command
		 * functions work in this mode -- the font and bitmap functions assume
		 * horizontal addressing.
		 */
		void ssd1306_tiny_init_vertical(void);

		/**
		 * @brief Begin an I2C data transmission.
		 *
		 * Sends the I2C start condition and the 0x40 control byte. After this,
		 * each byte sent with ssd1306_send_byte() goes into display RAM at the
		 * current cursor position.
		 *
		 * @see ssd1306_send_data_stop(), ssd1306_send_byte()
		 */
		void ssd1306_send_data_start(void);

		/**
		 * @brief End an I2C data transmission.
		 * @see ssd1306_send_data_start()
		 */
		void ssd1306_send_data_stop(void);

		/**
		 * @brief Send one raw byte over I2C.
		 *
		 * Must be called between a start/stop pair (either data or command).
		 * No framing is added -- just the byte itself.
		 *
		 * @param byte The byte to send.
		 */
		void ssd1306_send_byte(uint8_t byte);

		/**
		 * @brief Begin an I2C command transmission.
		 *
		 * Sends the I2C start condition and the 0x00 control byte. After this,
		 * each byte sent with ssd1306_send_byte() is interpreted as a command
		 * by the display controller.
		 *
		 * @see ssd1306_send_command_stop(), ssd1306_send_command()
		 */
		void ssd1306_send_command_start(void);

		/**
		 * @brief End an I2C command transmission.
		 * @see ssd1306_send_command_start()
		 */
		void ssd1306_send_command_stop(void);

		/**
		 * @brief Send a single command byte (start + byte + stop).
		 *
		 * Convenience wrapper when you only need to send one command.
		 *
		 * @param command The SSD1306 command byte.
		 */
		void ssd1306_send_command(uint8_t command);

		/**
		 * @brief Move the write cursor to a given position.
		 *
		 * @param x Column (0-127).
		 * @param y Page (0-7). Each page is 8 pixels tall.
		 */
		void ssd1306_setpos(uint8_t x, uint8_t y);

		/**
		 * @brief Fill the entire screen with a byte pattern.
		 *
		 * Writes 1024 bytes (128 columns x 8 pages). Pass 0x00 to clear the
		 * screen, 0xFF to turn every pixel on, or any other pattern.
		 *
		 * @param fill The byte pattern to repeat across the display.
		 */
		void ssd1306_fillscreen(uint8_t fill);

		/**
		 * @brief Set the display contrast (brightness).
		 *
		 * Controls the segment output current. Higher values draw more current
		 * and produce a brighter image. The init sequence sets this to 0x3F.
		 *
		 * Measured current draw on a typical 128x64 module with all pixels on:
		 * - 0x00: ~8-10 mA
		 * - 0xFF: ~20-25 mA
		 *
		 * @param value Contrast level (0x00 = dimmest, 0xFF = brightest).
		 */
		void ssd1306_set_contrast(uint8_t value);

		/**
		 * @brief Turn the display off (sleep mode).
		 *
		 * Puts the SSD1306 into sleep. Current draw drops to ~1 uA. Display
		 * RAM contents are preserved. Call ssd1306_display_on() to wake up.
		 */
		void ssd1306_display_off(void);

		/**
		 * @brief Turn the display on (wake from sleep).
		 *
		 * Resumes display output from RAM. Pair with ssd1306_display_off().
		 */
		void ssd1306_display_on(void);

#ifndef SSD1306_NO_FONT_6X8
		/**
		 * @brief Print one character using the 6x8 font.
		 *
		 * Draws at the current cursor position and advances the cursor 6
		 * columns to the right. The character occupies one page (8 pixels tall).
		 *
		 * @param ch ASCII character (32-127).
		 */
		void ssd1306_char_font6x8(char ch);

		/**
		 * @brief Print a null-terminated string using the 6x8 font.
		 *
		 * Draws at the current cursor position. Uses a single I2C data
		 * transaction for the whole string (faster than calling
		 * ssd1306_char_font6x8() per character). Does not wrap lines
		 * automatically.
		 *
		 * @param s Pointer to a null-terminated string.
		 */
		void ssd1306_string_font6x8(const char *s);
#endif

#ifndef SSD1306_NO_FONT_8X16
		/**
		 * @brief Print a string using the 8x16 font at a given position.
		 *
		 * Each character is 8 columns wide and spans two pages (16 pixels tall).
		 * If x > 120, wraps to the next line. The function draws the top half
		 * and bottom half of each character in two separate passes.
		 *
		 * @param x Starting column (0-127).
		 * @param y Starting page (0-7). The character occupies pages y and y+1.
		 * @param s Null-terminated string to print.
		 */
		void ssd1306_string_f8x16(uint8_t x, uint8_t y, const char s[]);
#endif

#ifndef SSD1306_NO_DRAW_BMP
		/**
		 * @brief Draw a page-aligned bitmap.
		 *
		 * The bitmap must be page-aligned (y coordinates are page numbers, not
		 * pixels). For pixel-level positioning, use ssd1306_draw_bmp_px() instead.
		 *
		 * @param x0 Starting column.
		 * @param y0 Starting page.
		 * @param x1 Ending column (exclusive).
		 * @param y1 Ending page (exclusive).
		 * @param bitmap Byte array stored in PROGMEM.
		 */
		void ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
#endif

		/**
		 * @brief Draw a bitmap at a pixel-level Y position.
		 *
		 * Unlike ssd1306_draw_bmp(), the Y coordinate is in pixels (0-63) rather
		 * than pages. The bitmap data is bit-shifted across page boundaries
		 * during transmission, so no RAM buffer is needed.
		 *
		 * X values >= 128 are ignored. If the bitmap extends past column 127,
		 * the extra columns are clipped.
		 *
		 * @warning Overwrites all 8 vertical pixels in each affected page strip.
		 *          Other content sharing those pages will be erased. The SSD1306
		 *          I2C interface does not support read-modify-write.
		 *
		 * @param x      Column (0-127). Values >= 128 are a no-op.
		 * @param y_px   Y position in pixels (0-63).
		 * @param w      Bitmap width in columns.
		 * @param h_pages Bitmap height in pages (1 page = 8 pixels).
		 * @param bitmap  Byte array stored in PROGMEM.
		 *
		 * @see ssd1306_clear_area_px(), ssd1306_draw_bmp_px_clipped()
		 */
		void ssd1306_draw_bmp_px(uint8_t x, uint8_t y_px, uint8_t w, uint8_t h_pages, const uint8_t bitmap[]);

		/**
		 * @brief Clear the area a pixel-positioned sprite occupies.
		 *
		 * Writes zeros to every page the sprite would touch at the given y_px.
		 * Call this before redrawing a sprite at a new position to avoid ghost
		 * pixels from the old position.
		 *
		 * @param x      Column (0-127). Values >= 128 are a no-op.
		 * @param y_px   Y position in pixels (0-63).
		 * @param w      Width in columns.
		 * @param h_pages Height in pages.
		 *
		 * @see ssd1306_draw_bmp_px()
		 */
		void ssd1306_clear_area_px(uint8_t x, uint8_t y_px, uint8_t w, uint8_t h_pages);

		/**
		 * @brief Draw a bitmap with signed X coordinate and automatic clipping.
		 *
		 * Same as ssd1306_draw_bmp_px() but X is a signed int16_t. Columns that
		 * fall outside 0-127 (left or right) are clipped. Use this when a sprite
		 * needs to slide smoothly on or off the screen edges.
		 *
		 * @param x       Column (can be negative).
		 * @param y_px    Y position in pixels (0-63).
		 * @param w       Full bitmap width in columns.
		 * @param h_pages Bitmap height in pages.
		 * @param bitmap  Byte array stored in PROGMEM.
		 *
		 * @see ssd1306_clear_area_px_clipped()
		 */
		void ssd1306_draw_bmp_px_clipped(int16_t x, uint8_t y_px, uint8_t w, uint8_t h_pages, const uint8_t bitmap[]);

		/**
		 * @brief Clear a sprite area with signed X clipping.
		 *
		 * Same as ssd1306_clear_area_px() but with signed X and automatic
		 * clipping. Pair with ssd1306_draw_bmp_px_clipped().
		 *
		 * @param x       Column (can be negative).
		 * @param y_px    Y position in pixels (0-63).
		 * @param w       Width in columns.
		 * @param h_pages Height in pages.
		 */
		void ssd1306_clear_area_px_clipped(int16_t x, uint8_t y_px, uint8_t w, uint8_t h_pages);

		/**
		 * @brief Composite one sprite into a page buffer using OR.
		 *
		 * When two sprites share a display page, drawing one normally erases the
		 * other (the SSD1306 has no read-back over I2C). Instead, composite both
		 * sprites into a buffer, then send the buffer once with ssd1306_send_buf().
		 *
		 * Typical workflow:
		 * @code
		 * uint8_t buf[16];
		 * memset(buf, 0, 16);  // zero before each frame
		 * SSD1306.ssd1306_compose_bmp_px(buf, 20, 16, 20, 20, 8, 1, sprA, 3);
		 * SSD1306.ssd1306_compose_bmp_px(buf, 20, 16, 20, 28, 8, 1, sprB, 3);
		 * SSD1306.ssd1306_send_buf(20, 3, buf, 16);
		 * @endcode
		 *
		 * @param buf           Caller-allocated buffer. Must be zeroed before
		 *                      compositing each frame.
		 * @param buf_x         Buffer's starting column on screen.
		 * @param buf_w         Buffer width in columns.
		 * @param sprite_x      Sprite X position (signed, for off-screen sprites).
		 * @param sprite_y_px   Sprite Y position in pixels.
		 * @param sprite_w      Sprite width in columns.
		 * @param sprite_h_pages Sprite height in pages.
		 * @param bitmap        Sprite pixel data in PROGMEM.
		 * @param target_page   Which display page (0-7) to composite into.
		 *
		 * @see ssd1306_send_buf()
		 */
		void ssd1306_compose_bmp_px(uint8_t *buf, uint8_t buf_x, uint8_t buf_w,
			int16_t sprite_x, uint8_t sprite_y_px,
			uint8_t sprite_w, uint8_t sprite_h_pages,
			const uint8_t bitmap[], uint8_t target_page);

		/**
		 * @brief Send a pre-composited buffer to the display.
		 *
		 * Writes the buffer contents to the display at the given column and page.
		 * Pair with ssd1306_compose_bmp_px().
		 *
		 * @param x    Starting column (0-127). Values >= 128 are a no-op.
		 * @param page Display page (0-7).
		 * @param buf  Pointer to the composited byte buffer.
		 * @param w    Buffer width in columns.
		 */
		void ssd1306_send_buf(uint8_t x, uint8_t page, const uint8_t *buf, uint8_t w);

	private:
		void I2CInit();
		bool I2CStart(uint8_t address);
		uint8_t I2CTransfer (uint8_t data);
		void I2CStop (void);
		bool I2CWrite(uint8_t data);
		void begin();
		void _ssd1306_start(uint8_t mode);
		void _ssd1306_init_from(const uint8_t *seq, uint8_t len);
};


/**
 * @brief Global driver instance.
 *
 * Use this to call all display functions. Do not create additional instances.
 *
 * @code
 * SSD1306.ssd1306_init();
 * SSD1306.ssd1306_string_font6x8("hello");
 * @endcode
 */
extern SSD1306Device SSD1306;

// ----------------------------------------------------------------------------

#endif
