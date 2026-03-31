/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "ssd1306xled", "index.html", [
    [ "Wiring (ATtiny85)", "index.html#autotoc_md1", null ],
    [ "Installation", "index.html#autotoc_md2", [
      [ "PlatformIO", "index.html#autotoc_md3", null ],
      [ "Arduino IDE", "index.html#autotoc_md4", null ],
      [ "Manual", "index.html#autotoc_md5", null ]
    ] ],
    [ "Quick start", "index.html#autotoc_md6", null ],
    [ "Documentation", "index.html#autotoc_md7", null ],
    [ "Changelog", "index.html#autotoc_md8", null ],
    [ "Contributing", "index.html#autotoc_md9", null ],
    [ "Credits", "index.html#autotoc_md10", null ],
    [ "License", "index.html#autotoc_md11", null ],
    [ "How the SSD1306 display works", "architecture.html", [
      [ "The display", "architecture.html#the_display", null ],
      [ "Using a 128x32 display", "architecture.html#display_128x32", null ],
      [ "Dual-color displays (yellow/blue)", "architecture.html#dual_color", null ],
      [ "Memory layout: pages", "architecture.html#memory_layout", null ],
      [ "Addressing modes", "architecture.html#addressing_modes", [
        [ "Horizontal addressing (default)", "architecture.html#horizontal_addressing", null ],
        [ "Vertical addressing", "architecture.html#vertical_addressing", null ],
        [ "Page addressing", "architecture.html#page_addressing", null ]
      ] ],
      [ "I2C protocol", "architecture.html#i2c_protocol", null ],
      [ "No read-back over I2C", "architecture.html#no_readback", null ],
      [ "Pixel-level positioning (the bit-shift trick)", "architecture.html#pixel_positioning", null ],
      [ "Initialization sequence", "architecture.html#init_sequence", null ],
      [ "I2C implementation", "architecture.html#i2c_implementation", null ]
    ] ],
    [ "Contributors", "contributors.html", [
      [ "Authors", "contributors.html#authors", null ],
      [ "Contributors", "contributors.html#contributor_list", null ]
    ] ],
    [ "Examples walkthrough", "examples.html", [
      [ "OLED_demo.ino", "examples.html#oled_demo", [
        [ "Setup", "examples.html#demo_setup", null ],
        [ "Fill patterns", "examples.html#demo_fill", null ],
        [ "Text rendering", "examples.html#demo_text", null ],
        [ "Image demo", "examples.html#demo_bitmap", null ],
        [ "Compositing demo", "examples.html#demo_compositing", null ]
      ] ],
      [ "sprite_overlap_fix.ino", "examples.html#sprite_overlap_fix", [
        [ "The bug", "examples.html#overlap_bug", null ],
        [ "The fix: compositing", "examples.html#overlap_fix", null ],
        [ "The fix: clipping", "examples.html#overlap_clipping", null ]
      ] ],
      [ "Creating your own bitmaps", "examples.html#creating_bitmaps", null ]
    ] ],
    [ "Features and flash usage", "features.html", [
      [ "Flash cost per feature", "features.html#flash_cost", null ],
      [ "Clipping – signed X coordinates", "features.html#clipping", null ],
      [ "Compositing – flicker-free overlapping sprites", "features.html#compositing", null ],
      [ "Build flags (PlatformIO only)", "features.html#build_flags", [
        [ "PlatformIO usage", "features.html#platformio_usage", null ],
        [ "Available flags", "features.html#available_flags", null ]
      ] ]
    ] ],
    [ "Getting started", "getting_started.html", [
      [ "What you need", "getting_started.html#what_you_need", null ],
      [ "Wiring", "getting_started.html#wiring", null ],
      [ "Installation", "getting_started.html#installation", [
        [ "PlatformIO", "getting_started.html#install_platformio", null ],
        [ "Arduino IDE", "getting_started.html#install_arduino", null ],
        [ "Manual", "getting_started.html#install_manual", [
          [ "Arduino IDE", "getting_started.html#autotoc_md12", null ],
          [ "PlatformIO", "getting_started.html#autotoc_md13", null ]
        ] ]
      ] ],
      [ "First program", "getting_started.html#first_program", null ],
      [ "Saving flash", "getting_started.html#saving_flash", null ]
    ] ],
    [ "Migrating from v0.x to v1.0", "migration.html", [
      [ "One thing you must change", "migration.html#rename", null ],
      [ "Bug fixes that change behavior", "migration.html#bug_fixes", [
        [ "X bounds checking on ssd1306_draw_bmp_px / ssd1306_clear_area_px", "migration.html#x_bounds", null ],
        [ "Uninitialized variable in ssd1306_string_f8x16", "migration.html#uninit_var", null ]
      ] ],
      [ "Conditional compilation guards", "migration.html#compilation_guards", null ],
      [ "New functions", "migration.html#new_functions", null ],
      [ "Flash savings", "migration.html#flash_savings", null ],
      [ "Version number", "migration.html#version_number", null ]
    ] ],
    [ "Wokwi simulation", "simulation.html", [
      [ "What you need", "simulation.html#sim_requirements", null ],
      [ "Setting up your OS", "simulation.html#sim_os_setup", [
        [ "macOS", "simulation.html#sim_macos", null ],
        [ "Ubuntu / Debian", "simulation.html#sim_ubuntu", null ],
        [ "Windows", "simulation.html#sim_windows", null ],
        [ "All platforms – Wokwi extension", "simulation.html#sim_all_platforms", null ]
      ] ],
      [ "Quick start", "simulation.html#sim_quickstart", null ],
      [ "Switching examples", "simulation.html#sim_examples", [
        [ "Available examples", "simulation.html#autotoc_md14", null ]
      ] ],
      [ "How it works", "simulation.html#sim_how", null ],
      [ "Links", "simulation.html#sim_links", null ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';