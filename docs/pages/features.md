# Feature flags {#features}

This library uses compile-time flags to include or exclude functionality.
Define them **before** `#include <ssd1306xled.h>`.

## Opt-in flags

These add extra features at the cost of some flash.

### SSD1306_CLIPPING -- signed X coordinates

```c
#define SSD1306_CLIPPING
#include <ssd1306xled.h>
```

Adds two functions:

- `ssd1306_draw_bmp_px_clipped(int16_t x, ...)` -- same as
  `ssd1306_draw_bmp_px()` but X is signed. Columns outside 0-127 are skipped.
- `ssd1306_clear_area_px_clipped(int16_t x, ...)` -- same idea for clearing.

Use this when you want a sprite to slide smoothly off the left or right edge
of the screen without having to bounds-check the coordinates yourself.

```c
// Sprite enters from the left
for (int16_t x = -16; x < 128; x++) {
    SSD1306.ssd1306_clear_area_px_clipped(x - 1, 20, 16, 2);
    SSD1306.ssd1306_draw_bmp_px_clipped(x, 20, 16, 2, sprite);
    _delay_ms(30);
}
```

### SSD1306_COMPOSITING -- flicker-free overlapping sprites

```c
#define SSD1306_COMPOSITING
#include <ssd1306xled.h>
```

Adds two functions:

- `ssd1306_compose_bmp_px(buf, ...)` -- OR a sprite's pixels into a
  caller-provided buffer for one display page.
- `ssd1306_send_buf(x, page, buf, w)` -- send the finished buffer to the
  display.

The problem: when two sprites share a display page, drawing one overwrites the
other's pixels (the SSD1306 has no read-back over I2C). Without compositing,
you see flicker as each sprite alternately erases the other.

The fix: composite both sprites into a small buffer, then write the buffer once.

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

### SSD1306_FAST_FILLSCREEN -- faster screen fill

```c
#define SSD1306_FAST_FILLSCREEN
#include <ssd1306xled.h>
```

Makes `ssd1306_fillscreen()` use a 4x-unrolled loop. Costs about 4 extra
bytes of flash but fills the screen faster. Worth it if you call fillscreen
frequently (like clearing the screen every frame).

## Opt-out flags

These exclude features to save flash. Most toolchains with GC sections already
drop unused functions at link time, but these flags give you guaranteed
exclusion regardless of toolchain settings.

| Flag                   | What it removes                                         | Flash saved |
|------------------------|---------------------------------------------------------|-------------|
| `SSD1306_NO_FONT_6X8` | 6x8 font data + `ssd1306_char_font6x8` + `ssd1306_string_font6x8` | ~582 bytes  |
| `SSD1306_NO_FONT_8X16` | 8x16 font data + `ssd1306_string_f8x16`                  | ~1570 bytes |
| `SSD1306_NO_DRAW_BMP`  | Page-aligned `ssd1306_draw_bmp` (use `ssd1306_draw_bmp_px` instead) | ~40 bytes   |

### SSD1306_QUICK_BEGIN

```c
#define SSD1306_QUICK_BEGIN
#include <ssd1306xled.h>
```

Skips the I2C device-present check during `begin()`. Normally the library
retries `I2CStart` in a loop until the display ACKs. This flag assumes the
display is already powered and ready. Saves ~50 bytes.

## Combination examples

Bitmap-only project (no text at all):

```c
#define SSD1306_NO_FONT_6X8
#define SSD1306_NO_FONT_8X16
#define SSD1306_NO_DRAW_BMP      // use ssd1306_draw_bmp_px instead
#define SSD1306_CLIPPING         // smooth edge scrolling
#define SSD1306_QUICK_BEGIN   // skip device check
#include <ssd1306xled.h>
```

Text-only project:

```c
#define SSD1306_NO_DRAW_BMP
#include <ssd1306xled.h>
```

Everything enabled (maximum functionality):

```c
#define SSD1306_CLIPPING
#define SSD1306_COMPOSITING
#define SSD1306_FAST_FILLSCREEN
#include <ssd1306xled.h>
```
