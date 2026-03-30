# Examples walkthrough {#examples}

## OLED_demo.ino

The demo sketch in `examples/OLED_demo/` exercises the main library features.
Here is what each section does.

### Setup

```c
void setup() {
    _delay_ms(40);
    SSD1306.ssd1306_init();
}
```

Waits 40 ms for the display to power up, then sends the initialization
sequence and clears the screen. After this, the display is ready.

### Fill patterns

```c
for (uint8_t i = 0; i < 8; i++) {
    SSD1306.ssd1306_fillscreen(i);
    _delay_ms(200);
}
SSD1306.ssd1306_fillscreen(0);
```

Fills the screen with byte patterns 0 through 7. Since each byte controls 8
vertical pixels, pattern 0x01 lights the top pixel in every column, 0x02 lights
the second pixel, and so on. The result is a quick animation of horizontal
lines moving down the screen. Then the screen is cleared.

### Text rendering

```c
SSD1306.ssd1306_setpos(0, 3);
SSD1306.ssd1306_string_font6x8("ssd1306xled");
_delay_ms(2000);
```

Moves the cursor to column 0, page 3 (roughly the middle of the screen) and
prints "ssd1306xled" using the 6x8 font. Each character is 6 pixels wide, so
11 characters take up 66 of the 128 columns.

### Bitmap display

```c
SSD1306.ssd1306_fillscreen(0);
SSD1306.ssd1306_draw_bmp(0, 0, 128, 8, logo);
_delay_ms(4000);
```

Clears the screen, then draws a full-screen bitmap stored in `image.h`. The
bitmap data is a `PROGMEM` array of 1024 bytes (128 columns x 8 pages). The
coordinates (0, 0, 128, 8) mean: start at the top-left corner and cover all
128 columns across all 8 pages.

A second bitmap from `image_two.h` is drawn the same way.

### Compositing demo

This section only compiles if you define `SSD1306_COMPOSITING` before including
the library:

```c
static const uint8_t spriteA[] PROGMEM = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF  // hollow rectangle
};
static const uint8_t spriteB[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // filled rectangle
};
```

Two 8x8 sprites are defined. Sprite A is a hollow rectangle (border only),
sprite B is a solid block.

```c
SSD1306.ssd1306_draw_bmp_px(20, 20, 8, 1, spriteA);  // page 2 portion
SSD1306.ssd1306_draw_bmp_px(20, 28, 8, 1, spriteB);  // page 4 portion
```

Each sprite is 1 page (8 pixels) tall. Sprite A at Y=20 touches pages 2 and 3.
Sprite B at Y=28 touches pages 3 and 4. Page 3 is the overlap zone -- both
sprites have bits there.

Drawing them normally would cause one to overwrite the other on page 3. The
compositing step fixes this:

```c
uint8_t buf[8];
memset(buf, 0, 8);
SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 20, 8, 1, spriteA, 3);
SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 28, 8, 1, spriteB, 3);
SSD1306.ssd1306_send_buf(20, 3, buf, 8);
```

Both sprites' contributions to page 3 are OR'd into the buffer, then the
buffer is sent to the display in a single write. Both sprites appear correctly.

## Creating your own bitmaps

Bitmap data for the SSD1306 is organized page-by-page, left to right, top to
bottom. Each byte represents 8 vertical pixels in one column (LSB = top).

You can create bitmap data using tools like:

- [image2cpp](https://javl.github.io/image2cpp/) -- online converter, outputs
  C arrays
- [LCD Assistant](http://en.radzio.dbd.pl/bitmap_converter/) -- Windows tool
- GIMP or Photoshop -- export as monochrome BMP, then convert

Make sure to select "vertical" byte orientation with LSB at top, and store the
result as a `PROGMEM` array.
