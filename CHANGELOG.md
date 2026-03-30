# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-03-30

### Fixed
- Sprites dropped down when drawn at negative X coordinates (issue #20). Added X bounds checking to `ssd1306_draw_bmp_px` and `ssd1306_clear_area_px`.
- Uninitialized variable in `ssd1306_string_f8x16` that could produce garbled text.

### Changed
- Renamed `ssd1306_char_f8x16` to `ssd1306_string_f8x16` -- the function takes a string, not a single character. **Breaking change.**
- Added `const` qualifier to `ssd1306_string_font6x8` parameter (`const char *s`).
- Removed dead code in `ssd1306_draw_bmp` (unused page calculation that was immediately overwritten).
- Removed unused I2C read path from `I2CStart` (~10 bytes flash, 2 bytes RAM saved).
- Merged duplicate I2C start functions into a shared internal helper (~12 bytes saved).
- Shared init helper removes duplicated init loops across init functions (~20 bytes saved).
- `ssd1306_string_font6x8` now uses a single I2C transaction per string instead of one per character (~15 bytes saved, also faster).
- Compact `ssd1306_fillscreen` loop (~4 bytes saved).

### Added
- `SSD1306_CLIPPING` opt-in flag: signed X clipping for smooth off-screen sprite entry/exit.
- `SSD1306_COMPOSITING` opt-in flag: page compositing for flicker-free overlapping sprites (issue #19).
- `SSD1306_FAST_FILLSCREEN` opt-in flag: 4x-unrolled fillscreen for faster fill operations.
- `SSD1306_NO_FONT_8X16` opt-out flag: exclude 8x16 font (~1570 bytes saved).
- `SSD1306_NO_FONT_6X8` opt-out flag: exclude 6x8 font (~582 bytes saved).
- `SSD1306_NO_DRAW_BMP` opt-out flag: exclude page-aligned `ssd1306_draw_bmp` (~40 bytes saved).
- Doxygen-based documentation site with API reference and guides.
- CHANGELOG.md (this file).

## [0.0.4] - 2026-03-15

### Added
- `ssd1306_draw_bmp_px()` for pixel-level vertical bitmap positioning. The bitmap data is bit-shifted across page boundaries during transmission, so no RAM buffer is needed.
- `ssd1306_clear_area_px()` for clearing pixel-positioned sprite areas without leaving ghost pixels.

## [0.0.3] - 2026-01-10

### Added
- `ssd1306_tiny_init_vertical()` to initialize the screen in vertical addressing mode.

## [0.0.2] - 2024-07-07

### Added
- `ssd1306_tiny_init()` to save flash memory when an initial screen fill is not needed (~52 bytes shorter than `ssd1306_init`).

## [0.0.1] - 2020-03-08

### Added
- Initial release with SSD1306, SSD1315, and SSH1106 support.
- Custom I2C bit-banging based on TinyI2C, replacing the Wire/TinyWireM dependency.
- Display functions: init, fillscreen, setpos, draw_bmp.
- Font rendering: 6x8 and 8x16 character sets.
- OLED demo example.
