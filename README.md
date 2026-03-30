# SSD1306XLED

This library is the driver for SSD1306, SSD1315 and SSH1106 based OLED screens. The original implementation is from Neven Boyanov, Tinusaur Team.

There were some compatibility issues with the I2C implementation of original `ssd1306xled` which I resolved by using the implementation from `TinyI2C` by David Johnson-Davies.

This library is tested on Attiny85 and some Chinese OLED screens based on these drivers.

### Connection
| OLED     | Attiny85 |
|----------|------|
| vcc      | vcc  |
| gnd      | gnd  |
| scl      | pb2  |
| sda      | pb0  |

![breadboard connection](images/breadboard.png?raw=true "Breadboard connection")

![schematic](images/schematic.png?raw=true "Schematic")

### Installation
- PlatformIO: `pio lib install "ssd1306xled"` or [visit library page](https://platformio.org/lib/show/7105/ssd1306xled/installation)
- Arduino IDE: open library manager (Tools > Manage library) and search for `ssd1306xled` and install
- Manual: Download the latest release [here](https://github.com/tejashwikalptaru/ssd1306xled/releases)

### How to use the code
- Hello world code
```c
#include <Arduino.h>
#include <ssd1306xled.h>

void setup() {
    _delay_ms(40);
    SSD1306.ssd1306_init();
}

void run() {
    SSD1306.ssd1306_fillscreen(0);
    SSD1306.ssd1306_setpos(0, 1);
    SSD1306.ssd1306_string_font6x8("Hello world!");
}
```

- Library functions
    - `void ssd1306_init(void)`: initializes the screen
    - `void ssd1306_tiny_init(void)`: initializes the screen without filling the screen with '0'
    - `void ssd1306_send_data_start(void)`: put the communication with a screen in data mode
    - `void ssd1306_send_data_stop(void)`: stops the communication
    - `void ssd1306_send_byte(uint8_t byte)`: send data byte to screen
    - `void ssd1306_send_command_start(void)`: put the communication with a screen in command mode
    - `void ssd1306_send_command_stop(void)`: stops the communication
    - `void ssd1306_send_command(uint8_t command)`: send command byte to screen
    - `void ssd1306_setpos(uint8_t x, uint8_t y)`: sets the cursor position at given coordinate
    - `void ssd1306_fillscreen(uint8_t fill)`: fill the screen with given data (provide 0 as an argument to clear screen)
    - `void ssd1306_char_font6x8(char ch)`: print a character with font size 6x8
    - `void ssd1306_string_font6x8(char *s)`: print a given string with font size 6x8
    - `void ssd1306_char_f8x16(uint8_t x, uint8_t y, const char ch[])`: print entire array with font size 8x16
    - `void ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[])`: draws the bitmap to the screen
    - `void ssd1306_draw_bmp_px(uint8_t x, uint8_t y_px, uint8_t w, uint8_t h_pages, const uint8_t bitmap[])`: draws a bitmap at a pixel-level y position (`y_px` is 0–63 in pixels, not pages). The bitmap data is bit-shifted across page boundaries during transmission so no RAM buffer is needed. Includes X bounds checking — values of `x` >= 128 are safely ignored. Note that this overwrites full 8-pixel page strips, so other content sharing those page rows will be erased.
    - `void ssd1306_clear_area_px(uint8_t x, uint8_t y_px, uint8_t w, uint8_t h_pages)`: clears the area occupied by a pixel-positioned sprite. Call this before redrawing at a new position to avoid leaving ghost pixels behind. Includes X bounds checking.

-  Special functions
    - `void ssd1306_tiny_init_vertical(void)`: initializes the screen in vertical addressing mode. Please note that at the moment the vertical addressing mode is *not* compatible with any other library functions than basic data and command transfer!

### Opt-in features

These features are enabled by defining the corresponding macro **before** including the library header. They add extra functionality at the cost of additional flash usage.

#### `SSD1306_CLIPPING` — Signed X coordinate support
Enables sprite drawing with signed X coordinates, allowing sprites to smoothly enter/exit the screen edges.

```c
#define SSD1306_CLIPPING
#include <ssd1306xled.h>
```

Functions added:
- `void ssd1306_draw_bmp_px_clipped(int16_t x, uint8_t y_px, uint8_t w, uint8_t h_pages, const uint8_t bitmap[])`: like `ssd1306_draw_bmp_px` but accepts negative X values. Columns that fall off-screen (left or right) are clipped automatically.
- `void ssd1306_clear_area_px_clipped(int16_t x, uint8_t y_px, uint8_t w, uint8_t h_pages)`: like `ssd1306_clear_area_px` with the same signed X clipping.

#### `SSD1306_COMPOSITING` — Page compositing for overlapping sprites
Enables merging multiple sprites that share the same display page, preventing flicker caused by full-page overwrites.

```c
#define SSD1306_COMPOSITING
#include <ssd1306xled.h>
```

Functions added:
- `void ssd1306_compose_bmp_px(uint8_t *buf, uint8_t buf_x, uint8_t buf_w, int16_t sprite_x, uint8_t sprite_y_px, uint8_t sprite_w, uint8_t sprite_h_pages, const uint8_t bitmap[], uint8_t target_page)`: composites a sprite's contribution for a specific page into a caller-provided buffer using OR. The buffer must be zeroed before compositing each frame.
- `void ssd1306_send_buf(uint8_t x, uint8_t page, const uint8_t *buf, uint8_t w)`: sends a composited buffer to the display.

Usage example — two sprites sharing page 3:
```c
uint8_t buf[16];                   // buffer width = sprite width
memset(buf, 0, 16);                // zero before each frame
SSD1306.ssd1306_compose_bmp_px(buf, 20, 16, 20, 20, 16, 1, spriteA, 3);
SSD1306.ssd1306_compose_bmp_px(buf, 20, 16, 20, 28, 16, 1, spriteB, 3);
SSD1306.ssd1306_send_buf(20, 3, buf, 16);  // single write, both sprites preserved
```

#### `SSD1306_FAST_FILLSCREEN` — Faster screen fill
Uses a 4x-unrolled loop for `ssd1306_fillscreen`, trading ~4 bytes of flash for faster fill operations.

```c
#define SSD1306_FAST_FILLSCREEN
#include <ssd1306xled.h>
```

### Opt-out feature flags

These flags exclude specific features from compilation to save flash. Define them **before** including the library header. Note: most toolchains with GC sections already drop unused functions at link time, but these flags provide guaranteed exclusion.

| Flag | What it excludes | Estimated savings |
|------|-----------------|-------------------|
| `SSD1306_NO_FONT_8X16` | 8x16 font data + `ssd1306_char_f8x16` | ~1570 bytes |
| `SSD1306_NO_FONT_6X8` | 6x8 font data + `ssd1306_char_font6x8` + `ssd1306_string_font6x8` | ~582 bytes |
| `SSD1306_NO_DRAW_BMP` | Page-aligned `ssd1306_draw_bmp` (use `ssd1306_draw_bmp_px` instead) | ~40 bytes |

Example for a bitmap-only project with no text rendering:
```c
#define SSD1306_NO_FONT_6X8
#define SSD1306_NO_FONT_8X16
#define SSD1306_NO_DRAW_BMP
#include <ssd1306xled.h>
```

### Contribution
You can improve this library by your contribution. If you want to improve the code or have a fix for some issues with the library, please feel free to fork this library and submit a new pull request with your changes and description

### Credits
This code is mainly written by Neven Boyanov, Tinusar team. I replaced their I2C implementation with the implementation by  David Johnson-Davies (TinyI2C).

### Versions
- v0.0.1 (March 08, 2020)
    - initial release
- v0.0.2 (July 07, 2024)
    - added 'ssd1306_tiny_init()' to save flash memory if an initial screen fill is not required (~52 bytes shorter)
- v0.0.3 (January 10, 2026)
    - added 'ssd1306_tiny_init_vertical()' to initialize the screen for vertical addresing mode
- v0.0.4 (March 15, 2026)
    - added 'ssd1306_draw_bmp_px()' for pixel-level vertical bitmap positioning
    - added 'ssd1306_clear_area_px()' for clearing pixel-positioned sprite areas
- v0.0.5 (upcoming)
    - **Bug fixes:**
        - Fixed sprites dropping down when drawn at negative X coordinates (issue #20) — X bounds checking added to `ssd1306_draw_bmp_px` and `ssd1306_clear_area_px`
        - Fixed uninitialized variable in `ssd1306_char_f8x16` that could cause garbled text rendering
    - **Core optimizations (automatic, no configuration needed):**
        - Removed unused I2C read path from I2CStart (~10 bytes flash, 2 bytes RAM saved)
        - Merged duplicate I2C start functions into shared internal helper (~12 bytes saved)
        - Shared init helper eliminates duplicated init loops across init functions (~20 bytes saved)
        - Optimized `ssd1306_string_font6x8` to use single I2C transaction per string instead of per-character (~15 bytes saved, also faster)
        - Compact `ssd1306_fillscreen` loop (~4 bytes saved)
    - **New opt-in features:**
        - `SSD1306_CLIPPING` — signed X clipping for smooth off-screen sprite entry/exit
        - `SSD1306_COMPOSITING` — page compositing for flicker-free overlapping sprites (issue #19)
        - `SSD1306_FAST_FILLSCREEN` — 4x-unrolled fillscreen for faster fill operations
    - **New opt-out flags for maximum flash savings:**
        - `SSD1306_NO_FONT_8X16` — exclude 8x16 font (~1570 bytes)
        - `SSD1306_NO_FONT_6X8` — exclude 6x8 font (~582 bytes)
        - `SSD1306_NO_DRAW_BMP` — exclude old page-aligned draw (~40 bytes)

