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
    ├── dictionary
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
| `src/core/` | Dictionary loading, rack matching, scoring, and result ranking |
| `src/platform/` | Runtime data discovery across development and install layouts |
| `src/ui/application.*` | GTK lifecycle and application-scoped resources |
| `src/ui/main_window.*` | Main workflow and coordination between UI and core |
| `src/ui/rack_view.*` | Visual representation of the seven-tile rack |
| `src/ui/result_list.*` | Rendering solver results without solving them |
| `src/ui/theme.*` | Application-wide GTK stylesheet loading |
| `tests/unit/` | Focused behavior tests corresponding to production modules |

## Ownership conventions

The code uses explicit ownership rules so future features do not rely on hidden
global state:

- `scrabble_dictionary_load()` returns an owned dictionary. Release it with
  `scrabble_dictionary_destroy()`.
- `scrabble_solve()` fills an owned result set. Release it with
  `scrabble_result_set_destroy()`.
- `scrabble_resource_find()` and `scrabble_resource_find_bundled()` return
  GLib-allocated paths. Release them with `g_free()`.
- GTK widgets own child widgets after they are appended or assigned.
- Application and window state are attached to their GTK owners with
  destruction callbacks.

## Resource discovery

Dictionary data is searched in this order:

1. `SCRABBLE_SOLVER_DATA_DIR` when explicitly set;
2. a portable `data/` folder beside the executable;
3. `../share/scrabble-solver/` relative to the executable;
4. the configured installation data directory; and
5. the source-tree `assets/` directory for development builds.

Bundled presentation resources such as the stylesheet can ignore an incomplete
dictionary override and fall back to normal application locations. This lets
the interface continue displaying a readable error state when custom data is
misconfigured.

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

## Adding a UI component

Reusable GTK components belong in their own `src/ui/<component>.c` and `.h`
pair. Components may format core data, but they should not reimplement rack,
dictionary, or scoring rules. Visual values belong in
`assets/styles/application.css` rather than scattered through C source.

## Future boundaries

Full board solving should be introduced as additional core modules rather than
being added to `main_window.c`. Likely boundaries include:

- board state and premium-square layout;
- move representation;
- cross-check generation;
- move validation;
- board-aware scoring; and
- candidate generation and ranking.

These modules can build on the current dictionary, rack, and scoring APIs while
remaining independently testable.
