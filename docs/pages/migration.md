# Migrating from v0.x to v1.0 {#migration}

Most sketches will need one find-and-replace and nothing else. This page
lists every change that could affect your code.

## One thing you must change

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

## Bug fixes that change behavior

### X bounds checking on ssd1306_draw_bmp_px / ssd1306_clear_area_px

In v0.0.4, passing `x >= 128` to these functions could cause the sprite to
wrap or draw in the wrong place. In v1.0.0, values of `x >= 128` return
immediately, and bitmaps extending past column 127 are clipped.

If your code relied on the wrapping behavior (unlikely), it will behave
differently now.

### Uninitialized variable in ssd1306_string_f8x16

The loop counter `i` was not initialized in some code paths, which could
garble text when using the 8x16 font. Fixed. If your text looked fine before,
nothing changes.

## Conditional compilation guards

The font and bitmap functions are now wrapped in `#ifndef` guards:

```c
#ifndef SSD1306_NO_FONT_6X8
    void ssd1306_char_font6x8(char ch);
    void ssd1306_string_font6x8(const char *s);
#endif
```

If you don't define any `SSD1306_NO_*` macros (the default), these guards
have no effect. Everything is included as before.

If you define `SSD1306_NO_FONT_6X8` and your sketch calls
`ssd1306_string_font6x8()`, you will get a compile error. That is by design.

## New opt-in features

Three features are available behind compile-time flags:

- **SSD1306_CLIPPING** -- `ssd1306_draw_bmp_px_clipped()` and
  `ssd1306_clear_area_px_clipped()` accept signed X coordinates.
- **SSD1306_COMPOSITING** -- `ssd1306_compose_bmp_px()` and
  `ssd1306_send_buf()` for merging overlapping sprites.
- **SSD1306_FAST_FILLSCREEN** -- 4x-unrolled fillscreen loop.

All off by default. Existing code is not affected unless you opt in.

See @ref features for details and code examples.

## Flash savings

The internals were refactored to share code between init functions and I2C
operations. Compiled binary size should drop by about 60 bytes compared to
v0.0.4. Nothing to do on your end.

## Version number

The jump from 0.0.4 to 1.0.0 reflects that the library has been in use since
2020 and the API is stable. The major version bump is not about breaking
changes -- it means the API is committed and the library is production-ready.
