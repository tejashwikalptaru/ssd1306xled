# Getting started {#getting_started}

## What you need

- An ATtiny85 (or any AVR with USI -- ATtiny25/45/85 all work)
- An SSD1306, SSD1315, or SSH1106 OLED display (128x64, I2C)
- PlatformIO or Arduino IDE

## Wiring

Connect four wires between the OLED breakout and the ATtiny85:

| OLED | ATtiny85 | Pin number |
|------|----------|------------|
| VCC  | VCC      | 8          |
| GND  | GND      | 4          |
| SCL  | PB2      | 7          |
| SDA  | PB0      | 5          |

@image html breadboard.png "Breadboard wiring"
@image html schematic.png "Schematic"

If your board uses different pins for I2C, override the defaults before
including the header:

```c
#define SSD1306_SCL PB3
#define SSD1306_SDA PB1
#include <ssd1306xled.h>
```

Some display modules use I2C address 0x3D instead of the default 0x3C. If your
screen stays blank after init, try:

```c
#define SSD1306_SA 0x3D
#include <ssd1306xled.h>
```

## Installation

### PlatformIO

```bash
pio lib install "ssd1306xled"
```

Or add it to `platformio.ini`:

```ini
lib_deps = ssd1306xled
```

### Arduino IDE

Open **Tools > Manage Libraries**, search for `ssd1306xled`, and click Install.

### Manual

Download the latest release from
[GitHub](https://github.com/tejashwikalptaru/ssd1306xled/releases), unzip it,
and place the folder in your Arduino libraries directory.

## First program

```c
#include <Arduino.h>
#include <ssd1306xled.h>

void setup() {
    _delay_ms(40);          // let the display power up
    SSD1306.ssd1306_init(); // send init sequence + clear screen
}

void loop() {
    SSD1306.ssd1306_setpos(0, 1);
    SSD1306.ssd1306_string_font6x8("Hello world!");
    _delay_ms(2000);
}
```

Line by line:

1. **`_delay_ms(40)`** -- The SSD1306 needs a moment after power-on before it
   can accept I2C commands. 40 ms is safe for most boards.
2. **`ssd1306_init()`** -- Sends the full initialization sequence (charge pump,
   clock, addressing mode, contrast, etc.) then fills the screen with zeros.
3. **`ssd1306_setpos(0, 1)`** -- Moves the cursor to column 0, page 1 (the
   second row of 8-pixel-tall characters).
4. **`ssd1306_string_font6x8("Hello world!")`** -- Prints the string using the
   built-in 6x8 font. Each character is 6 pixels wide and 8 pixels tall.

If you see nothing on the screen, double-check wiring and try swapping SDA/SCL.
If the display shows garbage, your module might use address 0x3D -- see the pin
customization section above.

## Saving flash

The ATtiny85 has 8 KB of flash. This library was built with that in mind, but
you can squeeze out more by excluding features you don't use.

If you only need bitmaps and no text:

```c
#define SSD1306_NO_FONT_6X8   // saves ~582 bytes
#define SSD1306_NO_FONT_8X16  // saves ~1570 bytes
#include <ssd1306xled.h>
```

If you use `ssd1306_draw_bmp_px()` and don't need the older page-aligned
`ssd1306_draw_bmp()`:

```c
#define SSD1306_NO_DRAW_BMP   // saves ~40 bytes
#include <ssd1306xled.h>
```

You can also skip the I2C device check at startup:

```c
#define SSD1306_QUICK_BEGIN // saves ~50 bytes
#include <ssd1306xled.h>
```

See the @ref features page for the full list of opt-in and opt-out flags.
