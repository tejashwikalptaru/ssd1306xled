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

-  Special functions
    - `void ssd1306_tiny_init_vertical(void)`: initializes the screen in vertical addressing mode. Please note that at the moment the vertical addressing mode is *not* compatible with any other library functions than basic data and command transfer!


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



