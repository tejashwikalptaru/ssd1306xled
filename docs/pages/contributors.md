# Contributors {#contributors}

This library exists because of the people listed here. Thanks to everyone who
has reported bugs, contributed code, or helped test new releases.

## Authors {#authors}

- **Neven Boyanov / [Tinusaur](https://bitbucket.org/tinusaur/ssd1306xled)** -- wrote the original SSD1306 driver.
- **David Johnson-Davies / [TinyI2C](https://github.com/technoblogy/tiny-i2c/)** -- wrote the I2C bit-banging implementation this library is based on.
- **[Tejashwi Kalp Taru](https://github.com/tejashwikalptaru)** -- current maintainer. Integrated the TinyI2C implementation, added pixel-level drawing, clipping, compositing, and compile-time feature flags.

## Contributors {#contributor_list}

- **[Lorandil](https://github.com/Lorandil)** -- code contributions and help.
- **[bkumanchik](https://github.com/bkumanchik)** -- reported the bugs that led to X bounds checking, compositing support, and the uninitialized variable fix (issues #19, #20).
- **[B3B3K](https://github.com/B3B3K)** -- figured out the init sequence changes for 128x32 displays (issue #10).
