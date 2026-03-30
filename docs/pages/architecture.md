# How the SSD1306 display works {#architecture}

This page explains the SSD1306 hardware and how the library talks to it. You
don't need to read this to use the library, but it helps when debugging display
glitches or writing your own drawing routines.

## The display

The SSD1306 is a single-chip driver for 128x64 monochrome OLED panels. It has
1 KB of internal display RAM (called GDDRAM) that maps directly to pixels on
screen. Write a byte to RAM, and the corresponding pixels change immediately.

There is no local framebuffer on the ATtiny85 side. Every draw call in this
library sends data straight to the display over I2C. The ATtiny85 only has 512
bytes of RAM, so buffering 1024 bytes of display data is not an option.

## Memory layout: pages

The display's 128x64 pixels are organized into **8 pages**. Each page is 128
bytes wide and 8 pixels tall:

```
Page 0: rows  0 -  7   (128 bytes)
Page 1: rows  8 - 15   (128 bytes)
Page 2: rows 16 - 23   (128 bytes)
...
Page 7: rows 56 - 63   (128 bytes)
```

Each byte in a page controls **8 vertical pixels** in a single column. The
least significant bit is the top pixel:

```
Bit 0 = top pixel    (row N)
Bit 1 = row N+1
Bit 2 = row N+2
...
Bit 7 = bottom pixel (row N+7)
```

So writing 0xFF to page 0, column 0 turns on all 8 pixels in the top-left
corner. Writing 0x01 turns on only the top pixel.

## Addressing modes

The SSD1306 supports three addressing modes. This library uses two of them.

### Horizontal addressing (default)

After writing a byte, the column pointer increments by 1. When it reaches
column 127, it wraps to column 0 on the next page. This is what
`ssd1306_init()` and `ssd1306_tiny_init()` set up.

With horizontal addressing, you can fill the entire screen by setting the
cursor to (0, 0) and sending 1024 bytes in one go. That is exactly what
`ssd1306_fillscreen()` does.

### Vertical addressing

After writing a byte, the page pointer increments by 1. When it reaches the
last page, it wraps to page 0 in the next column. Use
`ssd1306_tiny_init_vertical()` for this mode.

Vertical addressing is useful when your data is organized column-by-column
rather than row-by-row. The font and bitmap functions in this library assume
horizontal addressing, so they won't work after a vertical init.

### Page addressing

Not used by this library. Similar to horizontal but confined to a single page.

## I2C protocol

The SSD1306 communicates over I2C at address 0x3C (or 0x3D on some modules).
Every I2C transaction starts with a **control byte** that tells the display
how to interpret the bytes that follow:

- **0x00** -- command mode. Each following byte is an SSD1306 command (set
  contrast, set addressing mode, display on/off, etc.).
- **0x40** -- data mode. Each following byte goes into GDDRAM at the current
  cursor position.

The library wraps this as `ssd1306_send_command_start()` /
`ssd1306_send_data_start()`, followed by `ssd1306_send_byte()` calls, and
ending with `ssd1306_send_command_stop()` / `ssd1306_send_data_stop()`.

## No read-back over I2C

The SSD1306 does support reading GDDRAM back, but only over the parallel
interface. Over I2C, it is write-only. This has a practical consequence: the
library cannot read what is already on screen, modify a few bits, and write it
back.

When you draw a sprite with `ssd1306_draw_bmp_px()`, it overwrites all 8
vertical pixels in each affected page strip. If another sprite was sharing that
page, its pixels get wiped out.

The @ref features "compositing feature" (`SSD1306_COMPOSITING`) works around
this by merging sprites in a small RAM buffer before sending the combined
result to the display.

## Pixel-level positioning (the bit-shift trick)

The older `ssd1306_draw_bmp()` requires page-aligned Y coordinates (0, 1, 2,
..., 7). That limits vertical positioning to 8-pixel increments.

`ssd1306_draw_bmp_px()` takes a Y position in pixels (0-63). If the sprite
does not land on a page boundary, the function shifts the bitmap data across
page boundaries on the fly:

```
Example: sprite at y_px = 12 (page 1, offset 4)

Bitmap row 0 is split across pages 1 and 2:
  Page 1 gets: row_0 << 4  (lower 4 bits of row 0)
  Page 2 gets: row_0 >> 4  (upper 4 bits of row 0)
             | row_1 << 4  (lower 4 bits of row 1, OR'd in)
```

This happens during the I2C transmission, one column at a time. No RAM buffer
needed. The tradeoff is that it still overwrites full page strips -- you cannot
preserve the surrounding pixels in the same page.

## Initialization sequence

When you call `ssd1306_init()`, the library sends these commands:

| Command    | Purpose                                          |
|------------|--------------------------------------------------|
| 0xAE       | Display OFF (safe to configure)                  |
| 0xD5 0xF0  | Clock: max oscillator frequency                  |
| 0xA8 0x3F  | Multiplex ratio: 64 rows                         |
| 0xD3 0x00  | Display offset: 0                                |
| 0x40       | Start line: row 0                                |
| 0x8D 0x14  | Charge pump: enable (needed for most boards)     |
| 0x20 0x00  | Addressing: horizontal                           |
| 0xA1       | Segment remap: column 127 mapped to SEG0         |
| 0xC8       | COM scan direction: bottom to top                |
| 0xDA 0x12  | COM pins config: for 128x64 panel                |
| 0x81 0x3F  | Contrast: maximum                                |
| 0xD9 0x22  | Pre-charge period                                |
| 0xDB 0x20  | VCOMH deselect: 0.77 x VCC                       |
| 0xA4       | Output from RAM (not all-on test mode)           |
| 0xA6       | Normal display (not inverted)                    |
| 0x2E       | Deactivate scroll                                |
| 0xAF       | Display ON                                       |

The segment remap (0xA1) and COM scan (0xC8) together flip the display so that
(0, 0) is the top-left corner. Without them, the image appears mirrored and
upside-down on most breakout boards.

The charge pump (0x8D 0x14) generates the high voltage the OLED panel needs
from the 3.3V or 5V supply. Most breakout boards do not have an external VBAT
supply, so the internal charge pump is required.

## I2C implementation

This library bit-bangs I2C through the ATtiny85's USI (Universal Serial
Interface) peripheral rather than using the Wire or TinyWireM library. The USI
is a simple shift register that can be configured for two-wire mode.

The bit-banging approach saves about 1 KB of flash compared to Wire. The
tradeoff is that it is tied to AVR USI hardware -- it won't compile on ARM or
ESP boards without modification.

Clock speed defaults to fast mode (100-400 kHz). You can switch to standard
mode (up to 100 kHz) by commenting out `#define TWI_FAST_MODE` in the header.
