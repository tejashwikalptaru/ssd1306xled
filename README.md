# SSD1306XLED

This library is the driver for SSD1306, SSD1315 and SSH1106 based OLED screens. The original implementation is from Neven Boyanov, Tinusaur Team.

There was some compatibility issues with the I2C implementation of original `ssd1306xled` which I resolved by using implementation from `TinyI2C` by David Johnson-Davies.

This library is tested on Attiny85 and some chinese oled screens based on these drivers.

### Default pin connection
| Attiny85 | OLED |
|----------|------|
| vcc      | vcc  |
| gnd      | gnd  |
| scl      | pb2  |
| sda      | pb0  |