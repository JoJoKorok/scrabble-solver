# Contributing

Scrabble Solver is organized so the portable solving engine can evolve without
becoming coupled to GTK or operating-system details. Keep changes focused and
preserve the dependency direction described in
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Development workflow

1. Start from an up-to-date branch and make one cohesive change.
2. Add or update focused tests for behavior changes.
3. Build and test both the GTK application and the core-only configuration.
4. Keep compiler warnings enabled and resolve new warnings before committing.
5. Update the README, architecture document, roadmap, or changelog when the
   change affects users or future development.

Use short, imperative commit subjects with a conventional prefix such as
`feat:`, `fix:`, `test:`, `docs:`, `style:`, `ci:`, or `chore:`.

## Required checks

Run the full application checks with GCC warnings treated as errors:

```sh
cmake -S . -B build-check -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Werror" \
  -DSCRABBLE_BUILD_GUI=ON \
  -DBUILD_TESTING=ON
cmake --build build-check
ctest --test-dir build-check --output-on-failure
```

Then verify that the solver remains independent of GTK:

```sh
cmake -S . -B build-core-check -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Werror" \
  -DSCRABBLE_BUILD_GUI=OFF \
  -DBUILD_TESTING=ON
cmake --build build-core-check
ctest --test-dir build-core-check --output-on-failure
```

Pull requests should pass the Windows UCRT64 and Debian 13 jobs in
[the CI workflow](.github/workflows/ci.yml).

## Code and data guidelines

- Public core interfaces belong in `include/scrabble/`; implementation details
  stay private to their module.
- Core code must not include GTK or GLib headers.
- Allocation and cleanup APIs must be symmetrical and tested.
- UI components may present core results, but must not reimplement solver rules.
- Test fixtures should be small, deterministic, and committed with their tests.
- Do not add a third-party word list unless its license permits redistribution
  and its attribution requirements are documented.

## Versioning and releases

`project(VERSION ...)` in `CMakeLists.txt` is the single source of truth for the
application version. CMake generates the private version header used by the C
implementation.

For a release:

1. update the project version in `CMakeLists.txt`;
2. move completed entries from `Unreleased` in `CHANGELOG.md` into a dated
   version section;
3. run all required checks on Windows and Debian; and
4. create the matching `v<version>` tag only after the release commit is final.
