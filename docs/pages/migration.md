# Migrating from v0.x to v1.0 {#migration}

Most sketches will need one find-and-replace and nothing else. This page
lists every change that could affect your code.

## One thing you must change {#rename}

`ssd1306_char_f8x16` has been renamed to `ssd1306_string_f8x16`. The old
name was wrong -- the function prints a null-terminated string, not a single
character. The parameter was also renamed from `ch[]` to `s[]`.

```c
// Before
SSD1306.ssd1306_char_f8x16(0, 2, "Hello");

// After
SSD1306.ssd1306_string_f8x16(0, 2, "Hello");
```

Behavior is identical. Only the name changed. If you don't use the 8x16
font, this doesn't affect you.

## Bug fixes that change behavior {#bug_fixes}

### X bounds checking on ssd1306_draw_bmp_px / ssd1306_clear_area_px {#x_bounds}

In v0.0.4, passing `x >= 128` to these functions could cause the sprite to
wrap or draw in the wrong place. In v1.0.0, values of `x >= 128` return
immediately, and bitmaps extending past column 127 are clipped.

If your code relied on the wrapping behavior (unlikely), it will behave
differently now.

### Uninitialized variable in ssd1306_string_f8x16 {#uninit_var}

The loop counter `i` was not initialized in some code paths, which could
garble text when using the 8x16 font. Fixed. If your text looked fine before,
nothing changes.

## Conditional compilation guards {#compilation_guards}

The font and bitmap functions are wrapped in `#ifndef` guards that can
exclude them from compilation. These flags only work with PlatformIO's
`build_flags` (e.g. `-D SSD1306_NO_FONT_6X8` in `platformio.ini`).

They do **not** work as `#define` in Arduino IDE sketches. Arduino compiles
library `.cpp` files separately and does not pass sketch defines to them.
In Arduino IDE, the linker strips unused functions automatically, but font
data arrays remain in the binary.

See @ref features for the full list of build flags.

## New functions {#new_functions}

These are available without any flags -- just call them:

- `ssd1306_draw_bmp_px_clipped()` and `ssd1306_clear_area_px_clipped()` --
  signed X coordinates with automatic clipping at screen edges.
- `ssd1306_compose_bmp_px()` and `ssd1306_send_buf()` -- composite
  overlapping sprites into a buffer to avoid flicker.

The AVR linker strips any functions your sketch doesn't call, so these add
zero flash if unused.

See @ref features for details, code examples, and measured flash costs.

## Flash savings {#flash_savings}

The internals were refactored to share code between init functions and I2C
operations. The `ssd1306_fillscreen` loop is now 4x-unrolled by default.
Compiled binary size should drop compared to v0.0.4. Nothing to do on your end.

## Version number {#version_number}

The jump from 0.0.4 to 1.0.0 reflects that the library has been in use since
2020 and the API is stable. The major version bump is not about breaking
changes -- it means the API is committed and the library is production-ready.
