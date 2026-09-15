# Core API stability

This policy defines how the portable C interface may evolve without coupling
the solver engine to GTK or silently breaking its callers. The application and
core currently share one project version from `CMakeLists.txt`.

## Current status

Scrabble Solver is in the `0.x` development series. Public interfaces are
usable and tested, but a minor release may still contain a necessary breaking
source change. Patch releases must not intentionally break source compatibility.

The core is not currently distributed as a separately installed SDK or shared
library. External users may build and link `scrabble_core` from source, but they
must rebuild it with their application. No stable binary ABI is promised until
the project installs a versioned library and explicitly adopts an ABI policy.

## Public boundary

The public source API consists of:

- declarations under `include/scrabble/`;
- identifiers beginning with `scrabble_` or `Scrabble` declared there;
- public `SCRABBLE_` constants and status values; and
- the ownership, lifetime, error, and gameplay behavior documented by those
  headers.

Files under `src/core/`, including headers with `_internal` in their name, are
implementation details. Files under `src/ui/`, `src/platform/`, `tests/`,
`cmake/`, and `packaging/` are also outside the public core API. They may change
without a core migration note as long as the public contract remains intact.

Opaque types such as `ScrabbleBoard`, `ScrabbleDictionary`, and `ScrabbleGame`
hide their representation. Their create/destroy pairs and borrowed accessors
are part of the contract. Public value types such as `ScrabbleMove` and
`ScrabbleRack` expose their layout, so changing their fields, field order, or
size is a source-breaking change.

## Versioning rules

The project follows Semantic Versioning with these pre-1.0 clarifications:

| Release | Compatibility expectation |
|---|---|
| `0.y.z` patch | Bug fixes; no intentional source break |
| `0.y.0` minor | May include a documented source break while the API matures |
| `1.0.0` and later major | May remove or change existing public contracts |
| `1.x.0` minor | Backward-compatible public additions and deprecations |
| `1.x.y` patch | Backward-compatible fixes |

After 1.0, these changes require a new major version:

- removing or renaming a public function, type, field, constant, or enum value;
- changing a function signature or the meaning of its parameters;
- changing the layout of a public value type;
- renumbering existing status or enum values;
- changing whether returned memory is owned, borrowed, or mutable;
- shortening a documented lifetime or invalidating an existing call sequence;
- changing a public constant in a way that alters caller-owned storage; or
- changing documented solver or scoring semantics rather than correcting a bug.

Compatible minor releases may add functions, opaque types, or appended status
values. They must not add fields to an existing public value type unless that
type is explicitly versioned and designed for extension. Callers should retain
a default branch when handling status values from a newer library. Patch
releases may correct behavior that contradicts documented Scrabble rules; the
changelog must call out a correction that changes results.

## Ownership and lifetime

Ownership is part of source compatibility, not merely an implementation note.
Every function that allocates an object must identify its matching destroy
function. Borrowed pointers remain valid only for the lifetime documented by
their owning object, and callers must not free them.

New APIs should prefer opaque owning types when their representation may grow.
Public value types are appropriate for small data passed by value or filled by
the caller, but additions must consider initialization, size, and compatibility
before exposing a new field.

Thread safety is not implied. An object should be treated as confined to the
calling thread unless its public header explicitly says otherwise.

## Deprecation and migration

After 1.0, a replaceable API should normally be deprecated for the remainder of
its major version before removal. The replacement, changed ownership rules, and
a short conversion example belong in the public header or release notes.

During `0.x`, a breaking change may happen in a minor release without a full
deprecation cycle, but it must still include:

1. a `Changed` or `Removed` entry in `CHANGELOG.md`;
2. the old and new call pattern or type name;
3. any ownership or lifetime difference; and
4. focused tests for the replacement behavior.

Silent compatibility breaks are not acceptable in either phase.

## Review checklist

Before merging a change that touches `include/scrabble/`:

1. identify whether the change is additive, corrective, deprecated, or
   breaking;
2. preserve existing declarations when an additive alternative is practical;
3. review public structure layout, enum values, ownership, and pointer lifetime;
4. add unit and integration coverage for the public behavior;
5. record caller-visible changes and migration steps in the changelog; and
6. select a project version consistent with this policy before release.

The supported API is the behavior exercised by the current public headers and
tests, not undocumented details observed in a particular implementation.
