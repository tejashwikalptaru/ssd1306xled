# Features and flash usage {#features}

All functions are always compiled and available. The AVR linker strips
anything your sketch doesn't call, so unused features cost zero flash.

## Flash cost per feature {#flash_cost}

Measured on ATtiny85 with avr-gcc. These are the bytes each feature adds
when your sketch actually uses it:

| Feature | Flash cost | What it includes |
|---------|-----------|-----------------|
| Core (init + fillscreen + setpos) | 756 bytes | I2C driver, init sequence, screen fill |
| 6x8 font | +678 bytes | `ssd1306_char_font6x8`, `ssd1306_string_font6x8`, font data |
| 8x16 font | +1722 bytes | `ssd1306_string_f8x16`, font data |
| Page-aligned bitmap | +66 bytes | `ssd1306_draw_bmp` |
| Pixel-level bitmap | included in core | `ssd1306_draw_bmp_px`, `ssd1306_clear_area_px` |
| Signed X clipping | +136 bytes | `ssd1306_draw_bmp_px_clipped`, `ssd1306_clear_area_px_clipped` |
| Page compositing | +166 bytes | `ssd1306_compose_bmp_px`, `ssd1306_send_buf` |
| Contrast + display on/off | +68 bytes | `ssd1306_set_contrast`, `ssd1306_display_off`, `ssd1306_display_on` |

If you don't call a function, it doesn't end up in your binary.

## Clipping -- signed X coordinates {#clipping}

Draw and clear sprites with signed X coordinates. Columns outside 0-127 are
clipped automatically. Use this when a sprite needs to slide on or off the
screen edges.

```c
// Sprite enters from the left
for (int16_t x = -16; x < 128; x++) {
    SSD1306.ssd1306_clear_area_px_clipped(x - 1, 20, 16, 2);
    SSD1306.ssd1306_draw_bmp_px_clipped(x, 20, 16, 2, sprite);
    _delay_ms(30);
}
```

## Compositing -- flicker-free overlapping sprites {#compositing}

When two sprites share a display page, drawing one overwrites the other's
pixels (the SSD1306 has no read-back over I2C). Without compositing, you see
flicker as each sprite alternately erases the other.

The fix: composite both sprites into a small buffer, then write the buffer
once.

```c
// Two 8x8 sprites at Y=20 and Y=28.
// Both touch page 3 (rows 24-31).

uint8_t buf[8];
memset(buf, 0, 8);                                          // zero before each frame

SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 20, 8, 1, spriteA, 3);
SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 28, 8, 1, spriteB, 3);
SSD1306.ssd1306_send_buf(20, 3, buf, 8);                    // single write, both preserved
```

The buffer only needs to cover the columns where sprites overlap. For pages
that only one sprite touches, draw it normally with `ssd1306_draw_bmp_px()`.

## Build flags (PlatformIO only) {#build_flags}

These flags control compilation when passed via PlatformIO's `build_flags`.
PlatformIO passes `-D` flags to all compilation units, including library code.

**These flags do not work in Arduino IDE.** The Arduino IDE compiles library
`.cpp` files separately and does not pass defines from your sketch to library
compilation. In Arduino IDE, the linker already strips unused functions
automatically, so the main thing you miss is exclusion of font data arrays.

### PlatformIO usage {#platformio_usage}

In `platformio.ini`:

```ini
build_flags =
    -D SSD1306_NO_FONT_6X8
    -D SSD1306_NO_FONT_8X16
```

### Available flags {#available_flags}

| Flag | Effect |
|------|--------|
| `SSD1306_NO_FONT_6X8` | Exclude 6x8 font data and rendering functions from compilation |
| `SSD1306_NO_FONT_8X16` | Exclude 8x16 font data and rendering function from compilation |
| `SSD1306_NO_DRAW_BMP` | Exclude page-aligned `ssd1306_draw_bmp` from compilation (use `ssd1306_draw_bmp_px` instead) |
| `SSD1306_QUICK_BEGIN` | Skip the I2C device-present check during init. Saves 62 bytes. Assumes the display is already powered and ready |
