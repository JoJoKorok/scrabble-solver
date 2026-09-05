# Roadmap

This roadmap describes direction rather than fixed dates. Each milestone should
keep the core library portable and independently testable.

## 0.1 - Rack solver foundation

Status: complete

- Load and normalize a text dictionary.
- Match words against a seven-tile rack, including blanks.
- Calculate standard tile scores and rank results.
- Provide a GTK 4 desktop interface on Windows and Debian.
- Test core behavior and runtime resource discovery.

## 0.2 - Dictionary experience

Status: in progress

- [x] Bundle and attribute a redistributable word-game dictionary.
- [x] Let users select and remember a local dictionary file.
- [x] Report dictionary source, size, and loading problems in the interface.
- [ ] Improve search, filtering, and result navigation.
- [x] Define a documented import path for properly licensed word lists.

## 0.3 - Board model and scoring

Status: in progress

- [x] Represent the 15-by-15 board and premium-square layout in the core.
- [x] Display an accessible, selectable board in the desktop interface.
- [x] Place, score, undo, and reset a validated opening word manually.
- [x] Place a rack suggestion using the selected board square and direction.
- [ ] Validate tile placement, connectivity, and formed cross-words.
- [ ] Score main words, cross-words, premiums, blanks, and seven-tile bingos.
- [x] Add deterministic unit tests for completed board rules.

## 0.4 - Full move generation

Status: planned

- Generate legal placements from a rack and board position.
- Rank moves by total turn score.
- Add cancellation and progress reporting for longer searches.
- Profile realistic positions and optimize only measured bottlenecks.

## 1.0 - Supported desktop release

Status: planned

- Complete accessible board entry and move-result workflows.
- Package the GTK runtime and application resources for Windows.
- Produce an installable Debian package.
- Document supported platforms, dictionary setup, and troubleshooting.
- Establish a stable public core API and migration policy.
