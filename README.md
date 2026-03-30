# ssd1306xled

A lightweight driver for SSD1306, SSD1315, and SSH1106 OLED displays. Built for ATtiny85 and other memory-constrained AVR microcontrollers. Uses a custom I2C bit-banging implementation (based on [TinyI2C](https://github.com/technoblogy/tiny-i2c/)) instead of the Wire library, saving about 1 KB of flash.

Originally written by Neven Boyanov and the Tinusaur team.

## Wiring (ATtiny85)

| OLED | ATtiny85 |
|------|----------|
| VCC  | VCC      |
| GND  | GND      |
| SCL  | PB2      |
| SDA  | PB0      |

![breadboard connection](https://raw.githubusercontent.com/tejashwikalptaru/ssd1306xled/master/images/breadboard.png "Breadboard connection")

![schematic](https://raw.githubusercontent.com/tejashwikalptaru/ssd1306xled/master/images/schematic.png "Schematic")

## Installation

- **PlatformIO:** `pio lib install "ssd1306xled"` or [visit the library page](https://platformio.org/lib/show/7105/ssd1306xled/installation)
- **Arduino IDE:** Tools > Manage Libraries, search for `ssd1306xled`
- **Manual:** Download from the [releases page](https://github.com/tejashwikalptaru/ssd1306xled/releases)

## Quick start

```c
#include <Arduino.h>
#include <ssd1306xled.h>

void setup() {
    _delay_ms(40);
    SSD1306.ssd1306_init();
}

void loop() {
    SSD1306.ssd1306_fillscreen(0);
    SSD1306.ssd1306_setpos(0, 1);
    SSD1306.ssd1306_string_font6x8("Hello world!");
}
```

This clears the screen and prints "Hello world!" on the second row using the 6x8 font.

## Documentation

Full API reference, architecture guide, feature flags, and examples are available on the documentation site:

**[https://tejashwikalptaru.github.io/ssd1306xled/](https://tejashwikalptaru.github.io/ssd1306xled/)**

## Changelog

See [CHANGELOG.md](https://github.com/tejashwikalptaru/ssd1306xled/blob/master/CHANGELOG.md) for a complete version history.

## Contributing

Found a bug or want to add something? Fork the repo and open a pull request with your changes.

## Credits

- [Neven Boyanov / Tinusaur](https://bitbucket.org/tinusaur/ssd1306xled) -- original SSD1306 driver
- [David Johnson-Davies / TinyI2C](https://github.com/technoblogy/tiny-i2c/) -- I2C implementation

## License

[MIT](LICENSE)
