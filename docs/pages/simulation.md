# Wokwi simulation {#simulation}

You can test the library without any hardware using
[Wokwi](https://wokwi.com), a browser-based electronics simulator. The
`simulation/` directory in the repository has everything pre-configured:
a virtual ATtiny85 wired to an SSD1306 OLED, a PlatformIO build file, and
a Makefile that ties it together.

## What you need {#sim_requirements}

- [VS Code](https://code.visualstudio.com/) with the
  [Wokwi extension](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode)
- A free Wokwi license (one-time browser sign-up)
- Python 3 (the PlatformIO installer needs it)
- `make` (the simulation Makefile uses it)
- PlatformIO Core (`make setup` installs it for you)

## Setting up your OS {#sim_os_setup}

### macOS {#sim_macos}

Install the Xcode command-line tools. This gives you `make`, `git`, and
`python3` in one step:

```bash
xcode-select --install
```

If you prefer Homebrew:

```bash
brew install python3
```

(`make` ships with the Xcode command-line tools, so Homebrew won't install
it separately.)

VS Code can be installed from the website or with Homebrew:

```bash
brew install --cask visual-studio-code
```

### Ubuntu / Debian {#sim_ubuntu}

```bash
sudo apt update
sudo apt install -y build-essential python3 python3-venv git
```

`build-essential` includes `make` and `gcc`. VS Code is available as a
`.deb` download from the website, or through Snap:

```bash
sudo snap install code --classic
```

### Windows {#sim_windows}

**WSL2 (recommended)** -- install Ubuntu from the Microsoft Store, open it,
and follow the Ubuntu steps above. VS Code on the Windows side connects to
WSL automatically with the "Remote - WSL" extension.

**Native** -- install these separately:

- [Python 3](https://www.python.org/downloads/) (check "Add to PATH" during
  install)
- [Git](https://git-scm.com/download/win)
- `make` -- install through [Chocolatey](https://chocolatey.org/)
  (`choco install make`) or [Scoop](https://scoop.sh/) (`scoop install make`)
- [VS Code](https://code.visualstudio.com/)

### All platforms -- Wokwi extension {#sim_all_platforms}

1. Open VS Code, go to Extensions (Ctrl+Shift+X / Cmd+Shift+X), search for
   "Wokwi", and install it.
2. Press F1, type `Wokwi: Request a new License`, and follow the browser
   flow. This is free and only needs to be done once.

Then install PlatformIO from the `simulation/` directory:

```bash
cd simulation
make setup
```

This downloads and configures PlatformIO Core if it is not already installed.

## Quick start {#sim_quickstart}

```bash
cd simulation
make setup     # one-time: installs PlatformIO
make build     # compiles the default example for ATtiny85
```

Then open the `simulation/` folder in VS Code and press F1 ->
`Wokwi: Start Simulator`. The virtual OLED should light up and run through
the demo.

## Switching examples {#sim_examples}

The Makefile can build any example from the `examples/` directory. Pass the
folder name with `EXAMPLE=`:

```bash
make list                              # see what's available
make build EXAMPLE=OLED_demo           # build a different example
make build EXAMPLE=sprite_overlap_fix  # back to the default
make simulate                          # build + print next steps
```

`make clean` removes build artifacts and the `src` symlink.

### Available examples

| Example | What it shows |
|---------|---------------|
| `OLED_demo` | Fill patterns, text, bitmaps, and compositing |
| `sprite_overlap_fix` | The bug from [issue #19](https://github.com/tejashwikalptaru/ssd1306xled/issues/19): sprite flicker on shared pages, and the compositing/clipping fix |
| `invaders_fix` | Reproduces [issue #19](https://github.com/tejashwikalptaru/ssd1306xled/issues/19) with the reporter's game sprites: overlap bug, compositing fix, clipping fix, and a shooting demo with explosion animation |

## How it works {#sim_how}

The `simulation/` directory contains four files:

| File | Purpose |
|------|---------|
| `Makefile` | Wraps PlatformIO commands and handles example selection |
| `platformio.ini` | Tells PlatformIO to build for ATtiny85 using the library at the repo root |
| `diagram.json` | Wokwi circuit: ATtiny85 with SCL on PB2, SDA on PB0, wired to a 128x64 SSD1306 at address 0x3C |
| `wokwi.toml` | Points the Wokwi simulator at the compiled `.hex` and `.elf` files |

When you run `make build`, the Makefile symlinks `src/` to the chosen
example directory under `examples/`, then calls `pio run`. The compiled
firmware lands in `.pio/build/attiny85/`. The Wokwi extension reads
`wokwi.toml` to find that firmware and `diagram.json` to set up the virtual
circuit.

## Links {#sim_links}

- [Wokwi documentation](https://docs.wokwi.com/)
- [VS Code extension setup](https://docs.wokwi.com/vscode/getting-started)
- [Wokwi CI](https://docs.wokwi.com/wokwi-ci/getting-started)
- [diagram.json format](https://docs.wokwi.com/diagram-format)
- [SSD1306 part reference](https://docs.wokwi.com/parts/board-ssd1306)
