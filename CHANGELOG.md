# Changelog

Notable changes to Scrabble Solver are recorded in this file. The project uses
[Semantic Versioning](https://semver.org/) for numbered milestones.

## [Unreleased]

### Added

- A bundled, attributed ENABLE 2K word-game dictionary containing 169,266
  supported words.
- An integration test covering the `EEOIFNX` rack against the default
  dictionary.

### Planned

- User-selectable dictionary files.
- Full board-position representation and validation.
- Premium-square, cross-word, and bingo scoring.
- Ranked legal move generation.
- Self-contained Windows and Debian application packages.

## [0.1.0] - 2026-08-13

### Added

- Cross-platform C17 and GTK 4 desktop application structure.
- Rack matching with up to seven letters and blank-tile support.
- Dictionary loading, normalization, and duplicate removal.
- Standard English tile scoring with blank letters worth zero points.
- Ranked, scrollable word suggestions in a Scrabble-inspired interface.
- Portable runtime resource discovery and staged installation layout.
- Unit tests for core behavior and resource discovery.
- Warning-clean Windows UCRT64 and Debian 13 continuous integration builds.

[Unreleased]: https://github.com/JoJoKorok/scrabble-solver/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/JoJoKorok/scrabble-solver/releases/tag/v0.1.0
