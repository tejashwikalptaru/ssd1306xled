# How the SSD1306 display works {#architecture}

This page explains the SSD1306 hardware and how the library talks to it. You
don't need to read this to use the library, but it helps when debugging display
glitches or writing your own drawing routines.

## The display {#the_display}

The SSD1306 is a single-chip driver for 128x64 monochrome OLED panels. It has
1 KB of internal display RAM (called GDDRAM) that maps directly to pixels on
screen. Write a byte to RAM, and the corresponding pixels change immediately.

There is no local framebuffer on the ATtiny85 side. Every draw call in this
library sends data straight to the display over I2C. The ATtiny85 only has 512
bytes of RAM, so buffering 1024 bytes of display data is not an option.

## Using a 128x32 display {#display_128x32}

This library initializes for 128x64 by default. If you have a 128x32 module,
two registers need to change: the multiplex ratio (number of rows) and the COM
pins configuration. You can override them after init using the existing command
API -- no library modification needed:

```c
void setup() {
    _delay_ms(40);
    SSD1306.ssd1306_tiny_init();  // use tiny_init to skip the 1024-byte fill

    // Override for 128x32
    SSD1306.ssd1306_send_command_start();
    SSD1306.ssd1306_send_byte(0xA8);  // Set multiplex ratio
    SSD1306.ssd1306_send_byte(0x1F);  // 32 rows (0x1F = 31)
    SSD1306.ssd1306_send_byte(0xDA);  // Set COM pins configuration
    SSD1306.ssd1306_send_byte(0x02);  // Sequential COM, no remap (128x32)
    SSD1306.ssd1306_send_command_stop();

    SSD1306.ssd1306_fillscreen(0);    // clear the visible 4 pages
}
```

Use `ssd1306_tiny_init()` instead of `ssd1306_init()` to avoid the default
1024-byte screen fill. The 128x32 display only has 4 pages (512 bytes), so
the extra fill writes go to non-visible GDDRAM and waste I2C time.

The SSD1306 controller always has 1 KB of display RAM (8 pages) regardless
of the physical panel size. Pages 4-7 exist in memory but have no pixels on
a 128x32 module. Writing to them is harmless -- the data goes to RAM but
nothing shows on screen.

The drawing functions (`ssd1306_draw_bmp_px`, `ssd1306_setpos`, etc.) work
on a 128x32 display as long as you stay within pages 0-3 (rows 0-31). The
bounds checks in the library use page 7 as the maximum, which is fine -- the
hardware silently ignores pages that don't have physical pixels.

## Dual-color displays (yellow/blue) {#dual_color}

Some SSD1306 and SSD1315 modules are sold as "dual color" or "yellow/blue."
These have physically different phosphor material deposited on different rows
of the panel. The typical layout:

- **Rows 0-15 (pages 0-1):** yellow
- **Rows 16-63 (pages 2-7):** blue

This is a fixed property of the panel. The SSD1306 controller is monochrome
-- it only knows on/off per pixel and has no color-related commands. You cannot
change which rows are which color through software. No OLED library can,
because there is nothing to send over I2C.

What you can do is design your layout around the split. Put a title or status
bar in the top 16 rows (yellow zone) and your main content below. The
`ssd1306_setpos()` and `ssd1306_draw_bmp_px()` functions let you place content
at any row, so you have full control over what lands in which color zone.

The `ssd1306_set_contrast()` function affects the entire display uniformly --
you cannot set different brightness for the yellow and blue zones.

## Memory layout: pages {#memory_layout}

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

## Addressing modes {#addressing_modes}

The SSD1306 supports three addressing modes. This library uses two of them.

### Horizontal addressing (default) {#horizontal_addressing}

After writing a byte, the column pointer increments by 1. When it reaches
column 127, it wraps to column 0 on the next page. This is what
`ssd1306_init()` and `ssd1306_tiny_init()` set up.

With horizontal addressing, you can fill the entire screen by setting the
cursor to (0, 0) and sending 1024 bytes in one go. That is exactly what
`ssd1306_fillscreen()` does.

### Vertical addressing {#vertical_addressing}

After writing a byte, the page pointer increments by 1. When it reaches the
last page, it wraps to page 0 in the next column. Use
`ssd1306_tiny_init_vertical()` for this mode.

Vertical addressing is useful when your data is organized column-by-column
rather than row-by-row. The font and bitmap functions in this library assume
horizontal addressing, so they won't work after a vertical init.

### Page addressing {#page_addressing}

Not used by this library. Similar to horizontal but confined to a single page.

## I2C protocol {#i2c_protocol}

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

## No read-back over I2C {#no_readback}

The SSD1306 does support reading GDDRAM back, but only over the parallel
interface. Over I2C, it is write-only. This has a practical consequence: the
library cannot read what is already on screen, modify a few bits, and write it
back.

When you draw a sprite with `ssd1306_draw_bmp_px()`, it overwrites all 8
vertical pixels in each affected page strip. If another sprite was sharing that
page, its pixels get wiped out.

The compositing functions (`ssd1306_compose_bmp_px` and `ssd1306_send_buf`)
work around this by merging sprites in a small RAM buffer before sending the
combined result to the display. See the @ref features page for details.

## Pixel-level positioning (the bit-shift trick) {#pixel_positioning}

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

## Initialization sequence {#init_sequence}

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
| 0x81 0x3F  | Contrast: default (range 0x00-0xFF)              |
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

## I2C implementation {#i2c_implementation}

This library bit-bangs I2C through the ATtiny85's USI (Universal Serial
Interface) peripheral rather than using the Wire or TinyWireM library. The USI
is a simple shift register that can be configured for two-wire mode.

The bit-banging approach saves about 1 KB of flash compared to Wire. The
tradeoff is that it is tied to AVR USI hardware -- it won't compile on ARM or
ESP boards without modification.

Clock speed defaults to fast mode (100-400 kHz). You can switch to standard
mode (up to 100 kHz) by commenting out `#define TWI_FAST_MODE` in the header.
