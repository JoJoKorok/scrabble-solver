# Changelog

Notable changes to Scrabble Solver are recorded in this file. The project uses
[Semantic Versioning](https://semver.org/) for numbered milestones.

## [Unreleased]

### Added

- A bundled, attributed ENABLE 2K word-game dictionary containing 169,266
  supported words.
- An integration test covering the `EEOIFNX` rack against the default
  dictionary.
- Native custom dictionary selection with validation, remembered preferences,
  and a bundled-dictionary reset action.
- A private word-length index and repeatable full-dictionary performance test
  that keep rack searches responsive as dictionary support grows.
- A portable 15-by-15 board model with coordinate, tile, blank, center-square,
  and reset support for future move generation.
- A compact board-move representation with horizontal and vertical geometry,
  rack-versus-board tile tracking, and blank-tile identity.
- Atomic opening-move validation and application covering the center square,
  dictionary membership, rack letters, and blank assignment.
- An owned game state that distinguishes user and opponent turns, records
  visible opponent moves without their hidden rack, and supports safe undo and
  new-game reset while keeping the board synchronized.
- The standard premium-square layout and opening-move score breakdowns for
  letter premiums, word multipliers, blanks, and seven-tile bingo bonuses.
- A selectable GTK board view with coordinate labels, accessible square
  descriptions, permanent premium colors, and core-board refresh support.
- Manual opening-word controls for board position, direction, validated
  placement, score feedback, undo, and new-game reset.
- Opening placement directly from suggestions, with mouse and keyboard
  activation, shared validation, and explicit rack-versus-board score guidance.
- GTK integration tests covering suggestion ownership, blank scoring,
  placement validation, undo, and result invalidation, with a Debian CI display.
- Connected move validation for board intersections, adjacency, complete main
  words, perpendicular words, rack usage, and atomic placement.
- Full connected scoring for reused tiles, cross-words, premium squares,
  blanks, bingos, game history, and undo.
- Ranked legal board suggestions with exact coordinates and direction,
  highest-scoring blank assignment, direct placement, and full-dictionary
  performance coverage.
- End-to-end multi-turn gameplay coverage for suggested moves, board-edge
  placement, no-move racks, repeated undo, reset, and starting a fresh game.

### Planned

- Background cancellation and progress reporting for long searches.
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
