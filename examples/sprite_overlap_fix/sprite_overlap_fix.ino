/**
 * @file sprite_overlap_fix.ino
 * @brief Demonstrates the fix for issue #19 (sprite flicker on shared pages).
 *
 * The SSD1306 organises its 64-pixel height into eight 8-pixel "pages".
 * Because I2C is write-only (no read-back), drawing a sprite to a page
 * overwrites whatever was there before. When two sprites at different Y
 * positions share a page, the second draw erases the first -- causing
 * visible flicker.
 *
 * This example reproduces the exact scenario from issue #19:
 *   - Two rows of 16x8 invader sprites at Y=19 and Y=28.
 *     Both touch page 3 (rows 24-31), so naive drawing flickers.
 *   - A saucer sprite entering from off-screen (negative X).
 *
 * v1.0.0 fixes:
 *   - Compositing: OR sprites into a buffer for shared pages, send once.
 *   - Clipping: ssd1306_draw_bmp_px_clipped() handles negative X smoothly.
 *
 * @see https://github.com/tejashwikalptaru/ssd1306xled/issues/19
 */

#include <Arduino.h>
#include <ssd1306xled.h>
#include <font6x8.h>

// 16x8 invader sprite (1 page tall, 16 columns wide)
static const uint8_t invader[] PROGMEM = {
  0x70, 0x18, 0x7D, 0xB6, 0xBC, 0x3C, 0xBC, 0xB6,
  0xB6, 0xBC, 0x3C, 0xBC, 0xB6, 0x7D, 0x18, 0x70
};

// 16x8 saucer sprite
static const uint8_t saucer[] PROGMEM = {
  0x00, 0x00, 0x38, 0x7C, 0x56, 0xFE, 0x56, 0x7C,
  0x7C, 0x56, 0xFE, 0x56, 0x7C, 0x38, 0x00, 0x00
};

// ---------------------------------------------------------------------------
// Part 1: Show the flicker BUG (no compositing)
// ---------------------------------------------------------------------------
static void demo_broken() {
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 0);
  SSD1306.ssd1306_string_font6x8("BUG: flicker");

  // Draw two invader rows -- both touch page 3, so the second row's
  // write to page 3 erases the first row's contribution to that page.
  for (uint8_t i = 0; i < 3; i++) {
    // Redraw several times so the flicker is visible in the simulator.
    SSD1306.ssd1306_draw_bmp_px(10 + i * 40, 19, 16, 1, invader);  // row 1
    SSD1306.ssd1306_draw_bmp_px(10 + i * 40, 28, 16, 1, invader);  // row 2
  }
  _delay_ms(4000);
}

// ---------------------------------------------------------------------------
// Part 2: Show the FIX (compositing shared page 3)
// ---------------------------------------------------------------------------
static void demo_fixed() {
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 0);
  SSD1306.ssd1306_string_font6x8("FIX: compositing");

  for (uint8_t i = 0; i < 3; i++) {
    uint8_t x = 10 + i * 40;

    // Step 1: draw both sprites normally -- this handles all non-shared
    // pages (page 2 for row 1's top half, page 4 for row 2's bottom half).
    SSD1306.ssd1306_draw_bmp_px(x, 19, 16, 1, invader);  // row 1
    SSD1306.ssd1306_draw_bmp_px(x, 28, 16, 1, invader);  // row 2

    // Step 2: composite both sprites into a buffer for shared page 3,
    // then send the buffer once. This preserves both sprites' bits.
    uint8_t buf[16];
    memset(buf, 0, 16);
    SSD1306.ssd1306_compose_bmp_px(buf, x, 16, x, 19, 16, 1, invader, 3);
    SSD1306.ssd1306_compose_bmp_px(buf, x, 16, x, 28, 16, 1, invader, 3);
    SSD1306.ssd1306_send_buf(x, 3, buf, 16);
  }
  _delay_ms(4000);
}

// ---------------------------------------------------------------------------
// Part 3: Saucer entering from off-screen (negative X clipping)
// ---------------------------------------------------------------------------
static void demo_clipping() {
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 0);
  SSD1306.ssd1306_string_font6x8("FIX: clipping");

  // Saucer slides in from left edge (X goes from -16 to 112)
  for (int16_t x = -16; x <= 112; x += 2) {
    SSD1306.ssd1306_clear_area_px_clipped(x - 2, 3, 16, 1);
    SSD1306.ssd1306_draw_bmp_px_clipped(x, 3, 16, 1, saucer);
    _delay_ms(30);
  }
  _delay_ms(2000);
}

// ---------------------------------------------------------------------------

void setup() {
  _delay_ms(40);
  SSD1306.ssd1306_init();
}

void loop() {
  demo_broken();
  demo_fixed();
  demo_clipping();
}
