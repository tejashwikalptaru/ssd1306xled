/**
 * @file invaders_fix.ino
 * @brief Reproduces the exact bug from issue #19 using the reporter's sprites.
 *
 * bkumanchik reported two problems in their Space Invaders game:
 *
 *   1. Two invader rows at Y=19 and Y=28 flicker because both touch
 *      page 3 (rows 24-31). Drawing one row overwrites the other's
 *      pixels on that shared page.
 *
 *   2. A saucer starting at X=-16 glitches because ssd1306_draw_bmp_px
 *      takes an unsigned X. Negative values wrap to large positive ones.
 *
 * This example shows both bugs, then the fixes:
 *   - Compositing for overlapping rows (OR both into a buffer, send once)
 *   - Clipping for negative X (ssd1306_draw_bmp_px_clipped)
 *
 * The sprites are copied directly from the reporter's code.
 *
 * @see https://github.com/tejashwikalptaru/ssd1306xled/issues/19
 */

#include <Arduino.h>
#include <ssd1306xled.h>
#include <font6x8.h>
#include "sprites.h"

#define INV_COUNT   5
#define INV_SPACING 16
#define INV_W       16
#define ROW1_Y      19
#define ROW2_Y      28
#define SHARED_PAGE 3
#define TURRET_Y    56

// ---------------------------------------------------------------------------
// Part 1: Show the overlap BUG
// ---------------------------------------------------------------------------
static void demo_bug() {
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 0);
  SSD1306.ssd1306_string_font6x8("BUG: overlap");

  // Draw turret at bottom so the scene looks like the real game
  SSD1306.ssd1306_draw_bmp_px(60, TURRET_Y, INV_W, 1, turret);

  // Draw two rows of invaders without compositing.
  // Row 1 at Y=19 touches pages 2-3, row 2 at Y=28 touches pages 3-4.
  // The second row's write to page 3 erases row 1's contribution there.
  for (uint8_t i = 0; i < INV_COUNT; i++) {
    uint8_t x = 24 + i * INV_SPACING;
    SSD1306.ssd1306_draw_bmp_px(x, ROW1_Y, INV_W, 1, invaderA);
  }
  for (uint8_t i = 0; i < INV_COUNT; i++) {
    uint8_t x = 24 + i * INV_SPACING;
    SSD1306.ssd1306_draw_bmp_px(x, ROW2_Y, INV_W, 1, invader2A);
  }

  _delay_ms(5000);
}

// ---------------------------------------------------------------------------
// Part 2: Show the compositing FIX
// ---------------------------------------------------------------------------
static void demo_compositing() {
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 0);
  SSD1306.ssd1306_string_font6x8("FIX: compositing");

  SSD1306.ssd1306_draw_bmp_px(60, TURRET_Y, INV_W, 1, turret);

  for (uint8_t i = 0; i < INV_COUNT; i++) {
    uint8_t x = 24 + i * INV_SPACING;

    // Step 1: draw both rows normally.
    // This writes pages 2+3 for row 1 and pages 3+4 for row 2.
    // Page 3 gets corrupted, but pages 2 and 4 are correct.
    SSD1306.ssd1306_draw_bmp_px(x, ROW1_Y, INV_W, 1, invaderA);
    SSD1306.ssd1306_draw_bmp_px(x, ROW2_Y, INV_W, 1, invader2A);

    // Step 2: fix the shared page by compositing both rows into a buffer.
    uint8_t buf[INV_W];
    memset(buf, 0, INV_W);
    SSD1306.ssd1306_compose_bmp_px(buf, x, INV_W, x, ROW1_Y, INV_W, 1, invaderA,  SHARED_PAGE);
    SSD1306.ssd1306_compose_bmp_px(buf, x, INV_W, x, ROW2_Y, INV_W, 1, invader2A, SHARED_PAGE);
    SSD1306.ssd1306_send_buf(x, SHARED_PAGE, buf, INV_W);
  }

  _delay_ms(5000);
}

// ---------------------------------------------------------------------------
// Part 3: Show the clipping FIX for the saucer
// ---------------------------------------------------------------------------
static void demo_clipping() {
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 0);
  SSD1306.ssd1306_string_font6x8("FIX: clipping");

  // Draw some invaders and the turret so the scene has context
  for (uint8_t i = 0; i < INV_COUNT; i++) {
    uint8_t x = 24 + i * INV_SPACING;
    SSD1306.ssd1306_draw_bmp_px(x, ROW1_Y, INV_W, 1, invaderA);
    SSD1306.ssd1306_draw_bmp_px(x, ROW2_Y, INV_W, 1, invader2A);

    uint8_t buf[INV_W];
    memset(buf, 0, INV_W);
    SSD1306.ssd1306_compose_bmp_px(buf, x, INV_W, x, ROW1_Y, INV_W, 1, invaderA,  SHARED_PAGE);
    SSD1306.ssd1306_compose_bmp_px(buf, x, INV_W, x, ROW2_Y, INV_W, 1, invader2A, SHARED_PAGE);
    SSD1306.ssd1306_send_buf(x, SHARED_PAGE, buf, INV_W);
  }
  SSD1306.ssd1306_draw_bmp_px(60, TURRET_Y, INV_W, 1, turret);

  // Saucer slides in from off-screen left (X = -16) to right (X = 112).
  // ssd1306_draw_bmp_px_clipped handles negative X correctly.
  for (int16_t x = -16; x <= 112; x += 1) {
    SSD1306.ssd1306_clear_area_px_clipped(x - 1, 8, INV_W, 1);
    SSD1306.ssd1306_draw_bmp_px_clipped(x, 8, INV_W, 1, saucer);
    _delay_ms(20);
  }

  _delay_ms(2000);
}

// ---------------------------------------------------------------------------
// Helper: draw the full scene with compositing (invaders + turret)
// ---------------------------------------------------------------------------
static void draw_scene(uint8_t turret_x) {
  for (uint8_t i = 0; i < INV_COUNT; i++) {
    uint8_t x = 24 + i * INV_SPACING;
    SSD1306.ssd1306_draw_bmp_px(x, ROW1_Y, INV_W, 1, invaderA);
    SSD1306.ssd1306_draw_bmp_px(x, ROW2_Y, INV_W, 1, invader2A);

    uint8_t buf[INV_W];
    memset(buf, 0, INV_W);
    SSD1306.ssd1306_compose_bmp_px(buf, x, INV_W, x, ROW1_Y, INV_W, 1, invaderA,  SHARED_PAGE);
    SSD1306.ssd1306_compose_bmp_px(buf, x, INV_W, x, ROW2_Y, INV_W, 1, invader2A, SHARED_PAGE);
    SSD1306.ssd1306_send_buf(x, SHARED_PAGE, buf, INV_W);
  }
  SSD1306.ssd1306_draw_bmp_px(turret_x, TURRET_Y, INV_W, 1, turret);
}

// ---------------------------------------------------------------------------
// Part 4: Shooting with explosion animation
// ---------------------------------------------------------------------------
static void demo_shooting() {
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 0);
  SSD1306.ssd1306_string_font6x8("Shooting demo");

  // Draw invaders with compositing, turret starts under first target
  uint8_t first_inv_x = 24 + 1 * INV_SPACING;
  draw_scene((first_inv_x > 2) ? first_inv_x - 2 : 0);

  // Shoot three invaders one by one (columns 1, 2, 3 of row 2)
  for (uint8_t target = 1; target <= 3; target++) {
    uint8_t inv_x = 24 + target * INV_SPACING;
    // Center turret under the invader (invader is ~11px, turret is ~13px)
    uint8_t turret_x = (inv_x > 2) ? inv_x - 2 : 0;

    // Move turret to align with target
    SSD1306.ssd1306_clear_area_px(0, TURRET_Y, 128, 1);
    SSD1306.ssd1306_draw_bmp_px(turret_x, TURRET_Y, INV_W, 1, turret);

    // Bullet fires from the turret's gun tip (column 6 of the turret sprite)
    uint8_t shot_x = turret_x + 6;
    for (uint8_t y = 40; y > ROW2_Y; y -= 2) {
      SSD1306.ssd1306_clear_area_px(shot_x, y + 2, 8, 1);
      SSD1306.ssd1306_draw_bmp_px(shot_x, y, 8, 1, turShot);
      _delay_ms(25);
    }

    // Clear the bullet
    SSD1306.ssd1306_clear_area_px(shot_x, ROW2_Y, 8, 1);

    // Explosion: replace invader with blast sprite, flash 3 times
    for (uint8_t f = 0; f < 3; f++) {
      SSD1306.ssd1306_draw_bmp_px(inv_x, ROW2_Y, INV_W, 1, invBlast);

      // Recomposite page 3 so the explosion doesn't corrupt row 1
      uint8_t buf[INV_W];
      memset(buf, 0, INV_W);
      SSD1306.ssd1306_compose_bmp_px(buf, inv_x, INV_W, inv_x, ROW1_Y, INV_W, 1, invaderA, SHARED_PAGE);
      SSD1306.ssd1306_compose_bmp_px(buf, inv_x, INV_W, inv_x, ROW2_Y, INV_W, 1, invBlast,  SHARED_PAGE);
      SSD1306.ssd1306_send_buf(inv_x, SHARED_PAGE, buf, INV_W);

      _delay_ms(150);

      // Brief blank flash
      SSD1306.ssd1306_clear_area_px(inv_x, ROW2_Y, INV_W, 1);

      // Recomposite page 3 with row 1 only (row 2 slot now empty)
      memset(buf, 0, INV_W);
      SSD1306.ssd1306_compose_bmp_px(buf, inv_x, INV_W, inv_x, ROW1_Y, INV_W, 1, invaderA, SHARED_PAGE);
      SSD1306.ssd1306_send_buf(inv_x, SHARED_PAGE, buf, INV_W);

      _delay_ms(100);
    }

    // Clear the destroyed invader's spot for good
    SSD1306.ssd1306_clear_area_px(inv_x, ROW2_Y, INV_W, 1);
    // Fix page 3 one last time with just row 1
    uint8_t buf[INV_W];
    memset(buf, 0, INV_W);
    SSD1306.ssd1306_compose_bmp_px(buf, inv_x, INV_W, inv_x, ROW1_Y, INV_W, 1, invaderA, SHARED_PAGE);
    SSD1306.ssd1306_send_buf(inv_x, SHARED_PAGE, buf, INV_W);

    _delay_ms(500);
  }

  _delay_ms(2000);
}

// ---------------------------------------------------------------------------

void setup() {
  _delay_ms(40);
  SSD1306.ssd1306_init();
}

void loop() {
  demo_bug();
  demo_compositing();
  demo_clipping();
  demo_shooting();
}
