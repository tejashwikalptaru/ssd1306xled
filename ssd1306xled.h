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
 *
 */
#include <stdint.h>
#include <Arduino.h>
// #include <avr/pgmspace.h>
// #include <avr/interrupt.h>
#include <util/delay.h>

#ifndef SSD1306XLED_H
#define SSD1306XLED_H

// ----------------------------------------------------------------------------

// -----(+)--------------->	// Vcc,	Pin 1 on SSD1306 Board
// -----(-)--------------->	// GND,	Pin 2 on SSD1306 Board
#ifndef SSD1306_SCL
#define SSD1306_SCL		PB2	// SCL,	Pin 3 on SSD1306 Board
#endif
#ifndef SSD1306_SDA
#define SSD1306_SDA		PB0	// SDA,	Pin 4 on SSD1306 Board
#endif
#ifndef SSD1306_SA
#define SSD1306_SA		0X3C	// Slave address
#endif

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

// // Defines
#define TWI_FAST_MODE

#ifdef TWI_FAST_MODE                 // TWI FAST mode timing limits. SCL = 100-400kHz
#define DELAY_T2TWI (_delay_us(2))   // >1.3us
#define DELAY_T4TWI (_delay_us(1))   // >0.6us
#else                                // TWI STANDARD mode timing limits. SCL <= 100kHz
#define DELAY_T2TWI (_delay_us(5))   // >4.7us
#define DELAY_T4TWI (_delay_us(4))   // >4.0us
#endif

#define TWI_NACK_BIT 0 // Bit position for (N)ACK bit.

// Constants
// Prepare register value to: Clear flags, and set USI to shift 8 bits i.e. count 16 clock edges.
const unsigned char USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;
// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
const unsigned char USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;

#define SSD1306_COMMAND 0x00
#define SSD1306_DATA 0x40

// ----------------------------------------------------------------------------

class SSD1306Device
{
    public:
		SSD1306Device(void);
		void ssd1306_init(void);

		void ssd1306_send_data_start(void);
		void ssd1306_send_data_stop(void);
		void ssd1306_send_byte(uint8_t byte);

		void ssd1306_setpos(uint8_t x, uint8_t y);
		void ssd1306_fillscreen(uint8_t fill);
		void ssd1306_char_font6x8(char ch);
		void ssd1306_string_font6x8(char *s);
		void ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);

		void ssd1306_send_command_start(void);
		void ssd1306_send_command_stop(void);
		void ssd1306_send_command(uint8_t command);
		void ssd1306_char_f8x16(uint8_t x, uint8_t y, const char ch[]);

	private:
		int I2Ccount;
		void I2CInit();
		bool I2CStart(uint8_t address, int readcount);
		uint8_t I2CTransfer (uint8_t data);
		void I2CStop (void);
		bool I2CWrite(uint8_t data);
		void begin();
};


extern SSD1306Device SSD1306;

// ----------------------------------------------------------------------------

#endif