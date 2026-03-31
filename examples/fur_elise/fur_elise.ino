/**
 * @file fur_elise.ino
 * @brief Plays Fur Elise on PB4 buzzer and shows note names on the SSD1306.
 *
 * Demonstrates using the library for a scrolling text display while
 * generating sound through direct PORTB toggling on PB4 (same pin as
 * the game examples).
 *
 * The melody is stored in PROGMEM as pairs of (note_index, duration).
 * Note names scroll across the screen as each note plays.
 */

#include <Arduino.h>
#include <ssd1306xled.h>
#include <font6x8.h>

// ---------------------------------------------------------------------------
// Note frequencies (Hz). Index 0 = rest.
// ---------------------------------------------------------------------------

#define REST  0
#define C4    262
#define D4    294
#define DS4   311
#define E4    330
#define F4    349
#define G4    392
#define GS4   415
#define A4    440
#define B4    494
#define C5    523
#define D5    587
#define DS5   622
#define E5    659
#define F5    698
#define G5    784
#define A5    880
#define B5    988

// Note index table (PROGMEM) -- maps index to frequency
static const uint16_t note_freq[] PROGMEM = {
  0,    // 0: REST
  262,  // 1: C4
  294,  // 2: D4
  311,  // 3: DS4
  330,  // 4: E4
  349,  // 5: F4
  392,  // 6: G4
  415,  // 7: GS4
  440,  // 8: A4
  494,  // 9: B4
  523,  // 10: C5
  587,  // 11: D5
  622,  // 12: DS5
  659,  // 13: E5
  698,  // 14: F5
  784,  // 15: G5
  880,  // 16: A5
  988,  // 17: B5
};

// Note names for display (3 chars each, padded with space)
static const char note_names[][4] PROGMEM = {
  "-- ",  // 0: REST
  "C4 ",  // 1
  "D4 ",  // 2
  "Eb4",  // 3: DS4
  "E4 ",  // 4
  "F4 ",  // 5
  "G4 ",  // 6
  "G#4",  // 7: GS4
  "A4 ",  // 8
  "B4 ",  // 9
  "C5 ",  // 10
  "D5 ",  // 11
  "Eb5",  // 12: DS5
  "E5 ",  // 13
  "F5 ",  // 14
  "G5 ",  // 15
  "A5 ",  // 16
  "B5 ",  // 17
};

// ---------------------------------------------------------------------------
// Melody: Fur Elise opening theme
// Each entry is {note_index, duration_in_8ths}
// Duration: 1 = eighth note, 2 = quarter, 4 = half
// ---------------------------------------------------------------------------

static const uint8_t melody[] PROGMEM = {
  // Bar 1-2: E5 DS5 E5 DS5 E5 B4 D5 C5 A4
  13, 1,   12, 1,   13, 1,   12, 1,   13, 1,   9, 1,   11, 1,   10, 1,
  8, 2,

  // Bar 3-4: rest C4 E4 A4 B4
  0, 1,    1, 1,    4, 1,    8, 1,
  9, 2,

  // Bar 5-6: rest E4 GS4 B4 C5
  0, 1,    4, 1,    7, 1,    9, 1,
  10, 2,

  // Bar 7-8: rest E4 E5 DS5 E5 DS5 E5 B4 D5 C5
  0, 1,    4, 1,    13, 1,   12, 1,
  13, 1,   12, 1,   13, 1,   9, 1,    11, 1,   10, 1,

  // Bar 9-10: A4 rest C4 E4 A4 B4
  8, 2,
  0, 1,    1, 1,    4, 1,    8, 1,
  9, 2,

  // Bar 11-12: rest E4 C5 B4 A4
  0, 1,    4, 1,    10, 1,   9, 1,
  8, 2,

  // Repeat first phrase
  0, 1,
  13, 1,   12, 1,   13, 1,   12, 1,   13, 1,   9, 1,   11, 1,   10, 1,
  8, 2,
  0, 1,    1, 1,    4, 1,    8, 1,
  9, 2,
  0, 1,    4, 1,    7, 1,    9, 1,
  10, 2,
  0, 1,    4, 1,    13, 1,   12, 1,
  13, 1,   12, 1,   13, 1,   9, 1,    11, 1,   10, 1,
  8, 2,
  0, 1,    1, 1,    4, 1,    8, 1,
  9, 2,
  0, 1,    4, 1,    10, 1,   9, 1,
  8, 2,

  0xFF, 0xFF  // end marker
};

#define EIGHTH_MS  180  // tempo: milliseconds per eighth note
#define BUZZER_PIN PB4

// ---------------------------------------------------------------------------
// Sound: play a frequency on PB4 for a given duration (ms)
// ---------------------------------------------------------------------------

static void play_tone(uint16_t freq, uint16_t duration_ms) {
  if (freq == 0) {
    for (uint16_t i = 0; i < duration_ms; i++)
      _delay_ms(1);
    return;
  }

  // Use delayMicroseconds() for accurate variable-length delays.
  // Unlike a _delay_us(1) loop, this is a single calibrated call
  // per half-period with no per-iteration overhead.
  uint16_t half_period = 500000UL / freq;
  uint16_t cycles = ((uint32_t)freq * duration_ms) / 1000;

  for (uint16_t i = 0; i < cycles; i++) {
    PORTB |= (1 << BUZZER_PIN);
    delayMicroseconds(half_period);
    PORTB &= ~(1 << BUZZER_PIN);
    delayMicroseconds(half_period);
  }
}

// ---------------------------------------------------------------------------
// Display: scrolling note history
// ---------------------------------------------------------------------------

// Keep a rolling buffer of the last 10 note indices for display
#define HISTORY_LEN 6
static uint8_t history[HISTORY_LEN];
static uint8_t hist_pos;

static void display_init() {
  for (uint8_t i = 0; i < HISTORY_LEN; i++)
    history[i] = 0;
  hist_pos = 0;
}

static void display_note(uint8_t note_idx) {
  // Push new note into history
  history[hist_pos] = note_idx;
  hist_pos = (hist_pos + 1) % HISTORY_LEN;

  // Title
  SSD1306.ssd1306_setpos(16, 0);
  SSD1306.ssd1306_string_font6x8("Fur Elise");

  // Current note large in center
  char name[4];
  memcpy_P(name, note_names[note_idx], 4);
  SSD1306.ssd1306_setpos(46, 3);
  SSD1306.ssd1306_string_font6x8("    ");
  SSD1306.ssd1306_setpos(46, 3);
  SSD1306.ssd1306_string_font6x8(name);

  // Draw a simple bar showing pitch height (pages 5-6)
  // Map note index 0-17 to column width 0-100
  uint8_t bar_w = 0;
  if (note_idx > 0)
    bar_w = 6 * note_idx;
  if (bar_w > 120) bar_w = 120;

  // Clear bar area
  SSD1306.ssd1306_setpos(0, 5);
  SSD1306.ssd1306_send_data_start();
  for (uint8_t i = 0; i < 128; i++)
    SSD1306.ssd1306_send_byte(0);
  SSD1306.ssd1306_send_data_stop();

  // Draw bar
  SSD1306.ssd1306_setpos(4, 5);
  SSD1306.ssd1306_send_data_start();
  for (uint8_t i = 0; i < bar_w; i++)
    SSD1306.ssd1306_send_byte(0x3C);
  SSD1306.ssd1306_send_data_stop();

  // Scrolling history on bottom row (page 7)
  // Clear page 7 first to avoid wrap-around artifacts
  SSD1306.ssd1306_setpos(0, 7);
  SSD1306.ssd1306_send_data_start();
  for (uint8_t i = 0; i < 128; i++)
    SSD1306.ssd1306_send_byte(0);
  SSD1306.ssd1306_send_data_stop();

  SSD1306.ssd1306_setpos(4, 7);
  for (uint8_t i = 0; i < HISTORY_LEN; i++) {
    uint8_t idx = (hist_pos + i) % HISTORY_LEN;
    char n[4];
    memcpy_P(n, note_names[history[idx]], 4);
    SSD1306.ssd1306_string_font6x8(n);
    SSD1306.ssd1306_send_byte(0);
  }
}

// ---------------------------------------------------------------------------
// Setup & loop
// ---------------------------------------------------------------------------

void setup() {
  DDRB |= (1 << BUZZER_PIN);  // PB4 output for buzzer
  _delay_ms(40);
  SSD1306.ssd1306_init();
  display_init();
}

void loop() {
  SSD1306.ssd1306_fillscreen(0);

  uint16_t pos = 0;
  while (1) {
    uint8_t note_idx = pgm_read_byte(&melody[pos]);
    uint8_t dur_eighths = pgm_read_byte(&melody[pos + 1]);

    if (note_idx == 0xFF) break;  // end marker

    uint16_t freq = pgm_read_word(&note_freq[note_idx]);
    uint16_t duration_ms = (uint16_t)dur_eighths * EIGHTH_MS;

    display_note(note_idx);
    play_tone(freq, duration_ms);

    pos += 2;
  }

  // Pause before repeating
  SSD1306.ssd1306_setpos(34, 3);
  SSD1306.ssd1306_string_font6x8("       ");
  SSD1306.ssd1306_setpos(34, 3);
  SSD1306.ssd1306_string_font6x8("- END -");
  _delay_ms(3000);
}
