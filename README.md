# Scrabble Solver

Scrabble Solver is a cross-platform desktop application written in C17 with
GTK 4. Enter up to seven rack tiles and it displays the words that can be
formed, ranked by their standard English Scrabble tile score.

The application currently supports:

- uppercase or lowercase rack letters;
- `?` or `*` for blank tiles;
- zero-point scoring for letters supplied by blanks;
- ranked, scrollable word suggestions; and
- CMake build configurations for Windows and Debian.

This is currently a **rack solver**, not a full board-position solver. Board
placement, premium squares, cross-words, and move generation are planned for
later development.

The current development version is **0.1.0**. See [CHANGELOG.md](CHANGELOG.md)
for milestone details and [docs/ROADMAP.md](docs/ROADMAP.md) for the planned
path from rack solving to full board-position solving.

## Dictionary notice

The repository contains a deliberately small demonstration dictionary at
`assets/dictionaries/demo.txt`. It is useful for building and testing the
application, but it is not a tournament word list.

To use another dictionary, create a UTF-8 text file containing one alphabetic
word per line and either:

1. replace `assets/dictionaries/demo.txt`; or
2. create `dictionaries/demo.txt` inside another folder and set
   `SCRABBLE_SOLVER_DATA_DIR` to that folder before starting the application.

Only words of 1–15 ASCII letters are loaded. Words are normalized to uppercase,
invalid lines are ignored, and duplicate entries are removed.

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
        │   └── demo.txt
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
tests/support/          Lightweight shared C test utilities
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
