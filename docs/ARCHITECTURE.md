# Architecture

Scrabble Solver separates the portable solving engine from GTK and operating
system concerns. The core library must remain usable and testable when
`SCRABBLE_BUILD_GUI=OFF`.

## Dependency direction

```text
GTK application
    ├── UI components
    ├── resource discovery
    └── scrabble_core

scrabble_core
    ├── board
    ├── dictionary
    ├── game
    ├── move
    ├── placement
    ├── rack
    ├── scoring
    ├── solver
    └── version
```

Dependencies flow toward `scrabble_core`; the core never imports GTK or the UI.

## Module responsibilities

| Area | Responsibility |
|---|---|
| `include/scrabble/` | Public types and functions exposed by the core library |
| `src/core/` | Game and board state, move placement, dictionary loading, rack matching, scoring, and ranking |
| `src/platform/` | Runtime data discovery and persisted user settings |
| `src/ui/application.*` | GTK lifecycle and application-scoped resources |
| `src/ui/board_view.*` | Selectable rendering of board cells, premiums, and tiles |
| `src/ui/dictionary_picker.*` | Version-compatible native file selection |
| `src/ui/main_window.*` | Main workflow and coordination between UI and core |
| `src/ui/move_controls.*` | Opening-word, direction, undo, and new-game controls |
| `src/ui/rack_view.*` | Visual representation of the seven-tile rack |
| `src/ui/result_list.*` | Owning displayed result copies and emitting placement requests |
| `src/ui/theme.*` | Application-wide GTK stylesheet loading |
| `tests/integration/` | End-to-end portable workflows spanning solver, game, and board state |
| `tests/performance/` | Deterministic checks against the full bundled dictionary |
| `tests/unit/` | Focused behavior tests corresponding to production modules |
| `tests/ui/` | GTK workflow integration tests with isolated user settings |

## Ownership conventions

The code uses explicit ownership rules so future features do not rely on hidden
global state:

- `scrabble_dictionary_load()` returns an owned dictionary. Release it with
  `scrabble_dictionary_destroy()`.
- `scrabble_board_create()` returns an owned board. Release it with
  `scrabble_board_destroy()`.
- `scrabble_game_create()` returns an owned game containing its board and move
  history. Release it with `scrabble_game_destroy()`.
- `scrabble_solve()` fills an owned result set. Release it with
  `scrabble_result_set_destroy()`.
- `scrabble_resource_find()` and `scrabble_resource_find_bundled()` return
  GLib-allocated paths. Release them with `g_free()`.
- GTK widgets own child widgets after they are appended or assigned.
- Application and window state are attached to their GTK owners with
  destruction callbacks.

## Resource discovery and settings

The bundled dictionary is searched for in this order:

1. a portable `data/` folder beside the executable;
2. `../share/scrabble-solver/` relative to the executable;
3. the configured installation data directory; and
4. the source-tree `assets/` directory for development builds.

The main window first tries a custom dictionary path saved in the platform
user configuration directory. It validates the file before replacing the
active dictionary and falls back to the bundled dictionary if the saved file
is unavailable or invalid. The platform settings API accepts an explicit file
path so its persistence behavior can be tested without modifying real user
preferences.

Presentation resources such as the stylesheet are resolved independently of
the saved dictionary path. This lets the interface continue displaying a
readable error state when custom dictionary data is misconfigured.

## Adding a core feature

1. Add a focused public header under `include/scrabble/` only if the feature is
   part of the stable core API.
2. Add its implementation under `src/core/` and register it with
   `scrabble_core` in the root `CMakeLists.txt`.
3. Add a matching test file under `tests/unit/` and register it in
   `tests/CMakeLists.txt`.
4. Keep allocation and cleanup symmetrical and documented in the public API.
5. Verify both GUI and core-only configurations.

Private helpers shared only within an implementation area should use private
headers under that area rather than expanding the public API.

## Dictionary candidate index

The dictionary keeps its complete, alphabetically sorted word list for the
public lookup API. It also owns a private index that groups words of up to seven
letters by length, allowing opening-rack searches to avoid impossible lengths.
The board solver first filters the full list against the combined rack and
board letter inventory, then generates candidates only around occupied squares
and adjacent anchor squares. Full placement validation remains the final
authority, so the filters improve speed without changing legal results.

## Adding a UI component

Reusable GTK components belong in their own `src/ui/<component>.c` and `.h`
pair. Components may format core data, but they should not reimplement rack,
dictionary, or scoring rules. Visual values belong in
`assets/styles/application.css` rather than scattered through C source.

The board view copies cell values from a caller-owned `ScrabbleBoard` when it
is refreshed. It does not retain or modify the core board. Selection is UI
state, exposed through the component API so move-entry controls can coordinate
with it without knowing how the grid is rendered.

The `scrabble_ui` library is linked by both the executable and GTK integration
tests, so tests exercise the production components and main-window wiring.
Each result row owns a copy of its solver result and passes a temporary copy
to its callback; the solver can release its result set immediately. The main
window routes opening suggestions through the selected square and direction.
Connected results retain their validated move, including coordinates,
direction, and blank mask, and apply that exact move through the same scoring
and history path as manual entry. Rack, dictionary, and board changes discard
displayed results so stale placements cannot be applied accidentally.

For a local visual check, set `SCRABBLE_TEST_SCREENSHOT` to an absolute PNG
path before running the suggestion-placement test. GTK renders the tested
window after placement; normal test runs do not produce image files.

## Board-solving boundaries

Board solving is separated into independently testable core responsibilities:

- `move_analysis` identifies connectivity and every word formed;
- `placement` validates board and dictionary rules first, then layers rack
  ownership and automatic blank assignment onto player moves;
- `scoring` handles main words, cross-words, premiums, blanks, and bingos; and
- `solver` generates anchored candidates, optimizes blanks, and ranks moves.

This keeps GTK responsible only for presenting results and dispatching the
selected move. Cancellation and background execution can be added later
without moving Scrabble rules into the UI.

The rack-independent placement validators preserve explicit blank markers and
resolve existing board intersections without mutating the board. This shared
layer allows future known moves, such as an opponent's visible play, to use the
same geometry, connectivity, and dictionary authority without pretending the
tiles came from the user's rack.
