# Getting started {#getting_started}

## What you need {#what_you_need}

- An ATtiny85 (or any AVR with USI -- ATtiny25/45/85 all work)
- An SSD1306, SSD1315, or SSH1106 OLED display (128x64, I2C)
- PlatformIO or Arduino IDE

## Wiring {#wiring}

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

## Installation {#installation}

### PlatformIO {#install_platformio}

```bash
pio lib install "ssd1306xled"
```

Or add it to `platformio.ini`:

```ini
lib_deps = ssd1306xled
```

### Arduino IDE {#install_arduino}

Open **Tools > Manage Libraries**, search for `ssd1306xled`, and click Install.

### Manual {#install_manual}

Download the latest `.zip` from the
[releases page](https://github.com/tejashwikalptaru/ssd1306xled/releases).

#### Arduino IDE

Open **Sketch > Include Library > Add .ZIP Library** and select the downloaded
file. The IDE copies it into your libraries folder for you.

To do it by hand instead, unzip the archive and move the resulting folder to
your Arduino libraries directory:

- Linux / macOS: `~/Arduino/libraries/`
- Windows: `Documents\Arduino\libraries\`

Restart the IDE after adding the folder.

#### PlatformIO

Unzip the release into your project's `lib/` directory:

```
your_project/
  lib/
    ssd1306xled/
      ssd1306xled.h
      ssd1306xled.cpp
      font6x8.h
      font8x16.h
      library.json
```

PlatformIO picks up anything inside `lib/` automatically -- no extra
configuration needed.

## First program {#first_program}

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

## Saving flash {#saving_flash}

The ATtiny85 has 8 KB of flash. This library was built with that constraint in
mind. The AVR linker automatically strips any function your sketch does not
call, so unused features cost zero flash. You don't need to configure anything
for this to work.

If you use PlatformIO, you can go further by excluding font data arrays
entirely with `build_flags` in `platformio.ini`:

```ini
build_flags =
    -D SSD1306_NO_FONT_6X8
    -D SSD1306_NO_FONT_8X16
    -D SSD1306_QUICK_BEGIN
```

These flags do not work in Arduino IDE (the IDE compiles library files
separately and does not pass your sketch's defines to them). In Arduino IDE,
the linker handles the cleanup for you.

See the @ref features page for flash cost measurements and all available flags.
