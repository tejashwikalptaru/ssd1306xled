# Examples walkthrough {#examples}

## OLED_demo.ino {#oled_demo}

The demo sketch in `examples/OLED_demo/` runs through the main library
features. Each section is labeled on-screen so you can tell what's happening.

### Setup {#demo_setup}

```c
void setup() {
    _delay_ms(40);
    SSD1306.ssd1306_init();
}
```

Waits 40 ms for the display to power up, then sends the initialization
sequence and clears the screen.

### Fill patterns {#demo_fill}

```c
for (uint8_t i = 0; i < 8; i++) {
    SSD1306.ssd1306_fillscreen(i);
    _delay_ms(200);
}
```

Each byte in the display controls 8 vertical pixels (bit 0 = top, bit 7 =
bottom). Filling with 0x00 leaves the screen blank. Filling with 0x01 lights
the top pixel of every 8-pixel page strip, producing a thin horizontal line
every 8 rows. The loop runs through patterns 0-7, so you see a single line
stepping down one pixel at a time within each page.

### Text rendering {#demo_text}

```c
SSD1306.ssd1306_setpos(0, 0);
SSD1306.ssd1306_string_font6x8("Text demo");
SSD1306.ssd1306_setpos(0, 3);
SSD1306.ssd1306_string_font6x8("SSD1306XLED");
```

Prints a label at the top and the library name in the middle of the screen
using the 6x8 font. Each character is 6 pixels wide, so 11 characters take
66 of the 128 columns.

### Image demo {#demo_bitmap}

An "Image demo" label appears briefly, then two full-screen bitmaps are drawn
one after the other.

```c
SSD1306.ssd1306_fillscreen(0);
SSD1306.ssd1306_draw_bmp(0, 0, 128, 8, logo);
```

Each bitmap is a `PROGMEM` array of 1024 bytes (128 columns x 8 pages). The
coordinates (0, 0, 128, 8) cover all 128 columns across all 8 pages. The
first bitmap comes from `image.h`, the second from `image_two.h`.

### Compositing demo {#demo_compositing}

Two 8x8 sprites are defined: a hollow rectangle and a solid block.

```c
static const uint8_t spriteA[] PROGMEM = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF  // hollow rectangle
};
static const uint8_t spriteB[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // filled rectangle
};
```

Sprite A sits at Y=20 (pages 2-3) and sprite B at Y=28 (pages 3-4). Both
touch page 3.

**Without compositing** -- the demo first draws both sprites with plain
`ssd1306_draw_bmp_px()`. The second draw overwrites the first on page 3,
so one sprite's contribution to that page disappears. You can see the
broken result on-screen for a few seconds.

**With compositing** -- the demo then redraws using the compositing
functions. Non-shared pages are drawn normally. The shared page is built
in a buffer:

```c
uint8_t buf[8];
memset(buf, 0, 8);
SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 20, 8, 1, spriteA, 3);
SSD1306.ssd1306_compose_bmp_px(buf, 20, 8, 20, 28, 8, 1, spriteB, 3);
SSD1306.ssd1306_send_buf(20, 3, buf, 8);
```

Both sprites' bits for page 3 are OR'd into the buffer, then sent in one
write. Both sprites appear correctly this time.

## Creating your own bitmaps {#creating_bitmaps}

Bitmap data for the SSD1306 is organized page-by-page, left to right, top to
bottom. Each byte represents 8 vertical pixels in one column (LSB = top).

You can create bitmap data using tools like:

- [image2cpp](https://javl.github.io/image2cpp/) -- online converter, outputs
  C arrays
- [LCD Assistant](http://en.radzio.dbd.pl/bitmap_converter/) -- Windows tool
- GIMP or Photoshop -- export as monochrome BMP, then convert

Make sure to select "vertical" byte orientation with LSB at top, and store the
result as a `PROGMEM` array.
