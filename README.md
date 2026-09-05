# Scrabble Solver

Scrabble Solver is a cross-platform desktop application written in C17 with
GTK 4. Enter up to seven rack tiles and it displays the words that can be
formed, ranked by their standard English Scrabble tile score.

The application currently supports:

- uppercase or lowercase rack letters;
- `?` or `*` for blank tiles;
- zero-point scoring for letters supplied by blanks;
- a bundled 169,266-word North American English word-game dictionary;
- validated custom dictionary selection with a remembered preference;
- ranked, scrollable word suggestions;
- a selectable 15-by-15 board displaying the standard premium layout;
- manual validated opening-word placement with scoring, undo, and reset;
- a Place action on suggestions using the selected square and direction; and
- CMake build configurations for Windows and Debian.

This is currently a **rack solver with an in-progress board workflow**, not yet
a full board-position solver. The core can represent and score a validated
opening move, while connected board entry, cross-words, and legal move
generation are still being developed.

The current development version is **0.1.0**. See [CHANGELOG.md](CHANGELOG.md)
for milestone details and [docs/ROADMAP.md](docs/ROADMAP.md) for the planned
path from rack solving to full board-position solving.

## Place an opening suggestion

1. Enter your rack (use `?` or `*` for a blank) and click **Find words**.
2. Select the square for the word's first letter and choose **Horizontal** or
   **Vertical** in the opening-move controls.
3. Click **Place** beside a suggestion. You can also select a result and press
   Enter, or double-click it.

The word is checked against the current rack, dictionary, board boundaries,
and center star before placement. The listed score is the rack tile score;
the placement message includes board premiums and any seven-tile bonus.
Blanks are assigned by the core and displayed as zero-point board tiles.

After placement, use **Undo** to try another opening or **New game** to clear
the board. Suggestion placement is available only on an empty board for now.
The rack is still entered manually and is not reduced or refilled automatically.

## Dictionary

The default dictionary at `assets/dictionaries/enable2k.txt` is derived from
the public-domain ENABLE 2K word list. ENABLE was created as an open word-game
alternative to proprietary official lists. The bundled copy contains 169,266
unique words of 1–15 ASCII letters. Its source revision, transformation, and
attribution are documented in
[assets/dictionaries/README.md](assets/dictionaries/README.md).

ENABLE 2K is not the current NASPA Word List or Collins Scrabble Words list.
Accepted words can therefore differ from a particular commercial game,
tournament, region, or edition.

To use another dictionary, click **Choose file** in the application and select
a local word-list file. The app validates the file before replacing the active
dictionary and remembers a successful selection for the next launch. Click
**Use bundled** to return to ENABLE 2K and clear the saved preference.

The preference is stored in the platform user configuration directory:

- Windows: `%LOCALAPPDATA%\scrabble-solver\settings.ini`
- Debian: `${XDG_CONFIG_HOME:-~/.config}/scrabble-solver/settings.ini`

Only words of 1–15 ASCII letters are loaded. Words are normalized to uppercase,
invalid lines are ignored, and duplicate entries are removed. A selected file
must contain at least one supported word.

## Build on Debian

Install the compiler, CMake, Ninja, pkg-config implementation, and GTK 4
development files:

```sh
sudo apt update
sudo apt install build-essential cmake ninja-build pkgconf libgtk-4-dev
```

Configure, build, and test:

```sh
cmake -S . -B build -G Ninja \
  -DSCRABBLE_BUILD_GUI=ON \
  -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

GTK workflow tests need a display. On a headless Debian machine, install
`xvfb`, `xauth`, and `dbus`, then run
`xvfb-run -a ctest --test-dir build --output-on-failure`.
Without a display the GTK integration test is reported as skipped; the core
tests still run. The Debian CI job uses a virtual display. CMake runs the GTK
test in a private D-Bus session when `dbus-run-session` is available.

Run the application:

```sh
./build/scrabble_solver
```

The [`libgtk-4-dev`](https://packages.debian.org/libgtk-4-dev) package is
available in Debian 12 (Bookworm), Debian 13 (Trixie), and newer suites.

## Build on Windows

The supported Windows development environment is
[MSYS2](https://www.msys2.org/) using its **UCRT64** terminal. GTK’s
[official Windows installation guidance](https://www.gtk.org/docs/installations/windows/)
also recommends the MSYS2 package route for GCC development.

Open the UCRT64 terminal and install the required packages:

```sh
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-gtk4
```

From the project directory, configure, build, and test:

```sh
cmake -S . -B build -G Ninja \
  -DSCRABBLE_BUILD_GUI=ON \
  -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the application from the same UCRT64 terminal so the GTK runtime is on
`PATH`:

```sh
./build/scrabble_solver.exe
```

Do not mix the MSYS2 GTK libraries with a Visual Studio/MSVC build. The GTK
package above and the UCRT64 GCC toolchain use the same ABI and should be used
together.

## Core-only development

GTK is optional when working only on the solver engine. Disable the GUI while
keeping all core tests enabled:

```sh
cmake -S . -B build-core -G Ninja \
  -DSCRABBLE_BUILD_GUI=OFF \
  -DBUILD_TESTING=ON
cmake --build build-core
ctest --test-dir build-core --output-on-failure
```

## Install layout

Install into a staging directory by providing a prefix:

```sh
cmake --install build --prefix ./stage
```

The installation has this layout:

```text
stage/
├── bin/
│   └── scrabble_solver[.exe]
└── share/
    └── scrabble-solver/
        ├── dictionaries/
        │   ├── enable2k.txt
        │   └── README.md
        └── styles/
            └── application.css
```

Windows distribution will eventually require bundling the GTK runtime beside
the application. That packaging work is outside the current development build.

## Project structure

```text
.github/                Continuous integration and contribution templates
assets/                 Runtime dictionaries and styles
cmake/                  Generated build metadata templates
docs/                   Architecture and roadmap documentation
include/scrabble/       Stable public API for the solver engine
src/core/               Platform-independent solver implementation
src/platform/           Cross-platform resource discovery
src/ui/                 GTK application and reusable interface components
tests/fixtures/         Small deterministic test data
tests/performance/      Full-dictionary performance regression coverage
tests/support/          Lightweight shared C test utilities
tests/ui/               GTK workflow tests with isolated settings
tests/unit/             Focused tests that mirror production modules
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for module responsibilities,
ownership rules, and guidance for extending the project.

Contributions should follow [CONTRIBUTING.md](CONTRIBUTING.md), which documents
the required checks, architectural boundaries, and release process.

## Development checks

Before committing a change, run:

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```

To enable common GCC warnings in a fresh build:

```sh
cmake -S . -B build-warnings -G Ninja \
  -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic" \
  -DSCRABBLE_BUILD_GUI=ON \
  -DBUILD_TESTING=ON
cmake --build build-warnings
ctest --test-dir build-warnings --output-on-failure
```

The same GUI and core-only configurations run automatically on Windows UCRT64
and Debian 13 through [the CI workflow](.github/workflows/ci.yml).

## Name and trademark

Scrabble is a trademark of its respective owners. This independent project is
not affiliated with or endorsed by the trademark holders.
