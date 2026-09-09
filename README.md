# Scrabble Solver

Scrabble Solver is a cross-platform desktop application written in C17 with
GTK 4. Enter up to seven rack tiles to find opening words, then keep the board
updated to receive legal placements ranked by their complete turn score.

The application currently supports:

- uppercase or lowercase rack letters;
- `?` or `*` for blank tiles;
- zero-point scoring for letters supplied by blanks;
- a bundled 169,266-word North American English word-game dictionary;
- validated custom dictionary selection with a remembered preference;
- ranked, scrollable opening-word and legal-move suggestions;
- a selectable 15-by-15 board displaying the standard premium layout;
- validated opening and connected move placement with full Scrabble scoring;
- exact board coordinates and direction for connected suggestions;
- multi-turn game history with undo and reset; and
- CMake build configurations for Windows and Debian.

The app validates the main word and every perpendicular word made by a move.
It accounts for existing board letters, blanks, unused premium squares, and
seven-tile bonuses before ranking connected suggestions.

The current development version is **0.1.0**. See [CHANGELOG.md](CHANGELOG.md)
for milestone details and [docs/ROADMAP.md](docs/ROADMAP.md) for the planned
path from rack solving to full board-position solving.

## Solve successive turns

1. Enter your rack (use `?` or `*` for a blank) and click **Find words**.
2. Select the square for the word's first letter and choose **Horizontal** or
   **Vertical** in the opening-move controls.
3. Click **Place** beside a suggestion. You can also select a result and press
   Enter, or double-click it.
4. Replace the rack entry with the tiles for the next turn and click
   **Find words** again.
5. Choose a ranked legal move. Its row, column, and across/down direction are
   shown in the result, and **Place** applies that exact move to the board.

Every word is checked against the rack, dictionary, board boundaries, and
connection rules before placement. Opening choices show their rack score and
the placement message includes opening premiums. Later suggestions show their
complete board score, including cross-words and bonuses. For those later
suggestions, blanks are assigned to the highest-scoring valid positions and
displayed as zero-point tiles.

Use **Undo** to remove the last placed word without disturbing earlier turns,
or **New game** to clear the board. The rack is entered manually and is not
reduced or refilled automatically.

To keep the board synchronized after the other player acts, select the first
letter's square, choose the direction, enter their complete word, and click
**Record opponent move**. The app validates the visible play without using or
changing **Your Rack**. If the opponent placed a blank tile, enter its board
square under **Opponent blank squares** before recording the move. Separate two
blank squares with a space, such as `H9 J12`.

## Track a complete two-player game

Repeat this cycle after the opening word:

1. When the opponent plays, select their word's first square, enter the entire
   connected word and direction, mark any new blank squares, and click
   **Record opponent move**.
2. Replace **Your Rack** with the tiles currently on your rack and click
   **Find words**.
3. Choose and place one of the legal board suggestions. Then update the rack
   again after drawing replacement tiles.
4. Record the opponent's next visible move and continue the cycle.

Both players' moves share the same board and undo history. **Undo** removes the
most recently recorded move regardless of who played it, while **New game**
clears the complete board. The solver never needs to know the opponent's rack.

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
tests/integration/      End-to-end portable gameplay workflows
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
