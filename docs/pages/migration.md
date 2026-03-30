# Migrating from v0.x to v1.0 {#migration}

v1.0.0 is backwards-compatible for most projects. Existing sketches should
compile and work without changes. This page covers what is different and what
to watch for.

## Nothing to do (probably)

If your sketch uses the library with default settings -- no feature flags, no
custom macros -- it will compile and behave the same as before. The API has not
changed.

## Bug fixes that change behavior

### X bounds checking on ssd1306_draw_bmp_px / ssd1306_clear_area_px

In v0.0.4, passing `x >= 128` to these functions could cause the sprite to
wrap around or draw in the wrong place. In v1.0.0, values of `x >= 128` are
silently ignored (the function returns immediately). If the bitmap extends past
column 127, extra columns are clipped.

This is a bug fix, but if your code relied on the wrapping behavior (unlikely),
it will behave differently now.

### Uninitialized variable in ssd1306_char_f8x16

The loop counter `i` was not initialized in some code paths, which could cause
garbled text in the 8x16 font. This is now fixed. If your text looked fine
before, nothing changes.

## New conditional compilation guards

The font and bitmap functions are now wrapped in `#ifndef` guards:

```c
#ifndef SSD1306_NO_FONT_6X8
    void ssd1306_char_font6x8(char ch);
    void ssd1306_string_font6x8(char *s);
#endif
```

If you do not define any `SSD1306_NO_*` macros (which is the default), these
guards have no effect. The functions are included as before.

If you define `SSD1306_NO_FONT_6X8` and your sketch calls
`ssd1306_string_font6x8()`, you will get a compile error. That is intentional
-- the flag exists to exclude the function.

## New opt-in features

Three new features are available if you define the corresponding macros before
including the header:

- **SSD1306_CLIPPING** -- `ssd1306_draw_bmp_px_clipped()` and
  `ssd1306_clear_area_px_clipped()` accept signed X coordinates.
- **SSD1306_COMPOSITING** -- `ssd1306_compose_bmp_px()` and
  `ssd1306_send_buf()` for merging overlapping sprites.
- **SSD1306_FAST_FILLSCREEN** -- 4x-unrolled fillscreen loop.

These are off by default. Your existing code is not affected unless you opt in.

See @ref features for details and code examples.

## Flash savings from core optimizations

The internals were refactored to share code between init functions and I2C
operations. If you are checking compiled binary size, you should see a net
decrease of about 60 bytes compared to v0.0.4. No action needed on your part.

## Version number jump

The version goes from 0.0.4 to 1.0.0. The library has been in use for several
years and the API is stable, so a 1.0 release felt overdue. The major version
bump does not signal a breaking change -- it signals that the library is
production-ready and the API is committed.
